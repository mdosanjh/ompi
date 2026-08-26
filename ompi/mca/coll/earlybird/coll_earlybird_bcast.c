/*
 * Copyright (c) 2026 Sandia National Laboratories. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include "opal/util/mprintf.h"

/*
 * Broadcast Direct RMA Init
 */
int mca_coll_earlybird_bcast_init(void *buffer, size_t count, struct ompi_datatype_t *datatype,
                                 int root, struct ompi_communicator_t *comm, struct ompi_info_t *info,
                                 ompi_request_t **request, struct mca_coll_base_module_t *module)
{
    mca_coll_earlybird_req_t *req;
    size_t total_bytes;

    /* Validate contiguous datatype */
    if (!mca_coll_earlybird_is_contiguous(datatype)) {
        return OMPI_ERROR;
    }

    total_bytes = count * ompi_datatype_get_size(datatype);

    req = OBJ_NEW(mca_coll_earlybird_req_t);
    if (!req) return OMPI_ERROR;

    ompi_request_init(&req->super, true);
    req->super.req_type = OMPI_REQUEST_COLL;

    req->sbuf = buffer;
    req->scount = count;
    req->sdtype = datatype;
    req->comm = comm;
    req->user_recvbuf = buffer;
    req->root = root;

    /* Every rank needs a shadow buffer and window to receive data (including root) */
    if (mca_coll_earlybird_setup_window(req, total_bytes, comm) != OMPI_SUCCESS) {
        OBJ_RELEASE(req);
        return OMPI_ERROR;
    }

    OPAL_OUTPUT_VERBOSE((10, ompi_coll_base_framework.framework_output,
                        "EarlyBird: Bcast init - root %d, shadow_buf allocated (%zu bytes)",
                        root, total_bytes));

    *request = &req->super;
    return OMPI_SUCCESS;
}

/*
 * Broadcast Direct RMA Operation (Start)
 */
int mca_coll_earlybird_bcast_start(size_t count, struct ompi_request_t **requests)
{
    for (size_t i = 0; i < count; i++) {
        mca_coll_earlybird_req_t *req = (mca_coll_earlybird_req_t *)requests[i];
        if (!req || req->super.req_type != OMPI_REQUEST_COLL) continue;

        int rank;
        ompi_comm_rank(req->comm, &rank);

        if (rank == req->root) {
            int size;
            ompi_comm_size(req->comm, &size);
            size_t send_bytes = req->scount * ompi_datatype_get_size(req->sdtype);

            /* Root puts data to every peer's shadow buffer */
            for (int j = 0; j < size; j++) {
                MPI_Put(req->sbuf, send_bytes, MPI_BYTE, j, 0, 
                        send_bytes, MPI_BYTE, req->win);
            }
        }
        /* Non-root ranks are passive targets for the Root's puts */
    }
    return OMPI_SUCCESS;
}

/*
 * Broadcast Direct RMA Wait
 */
void mca_coll_earlybird_bcast_wait(ompi_request_t *req_base)
{
    mca_coll_earlybird_req_t *req = (mca_coll_earlybird_req_t *)req_base;
    
    /* 1. Ensure all RMA puts from root are complete and visible */
    MPI_Win_fence(0, req->win);
    
    /* 2. Copy from shadow buffer to user buffer */
    memcpy(req->user_recvbuf, req->shadow_buf, req->shadow_buf_size);
    
    /* 3. Mandatory final fence to restore window consistency for reuse */
    MPI_Win_fence(0, req->win);
}

/*
 * Broadcast Direct RMA Free
 */
void mca_coll_earlybird_bcast_free(ompi_request_t *req_base)
{
    mca_coll_earlybird_req_t *req = (mca_coll_earlybird_req_t *)req_base;
    
    mca_coll_earlybird_cleanup_window(req);
    OBJ_RELEASE(req);
}
