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
 * Gather Direct RMA Init
 */
int mca_coll_earlybird_gather_init(const void *sendbuf, size_t sendcount, struct ompi_datatype_t *sendtype,
                                  void *recvbuf, size_t recvcount, struct ompi_datatype_t *recvtype,
                                  int root, struct ompi_communicator_t *comm, struct ompi_info_t *info,
                                  ompi_request_t **request, struct mca_coll_base_module_t *module)
{
    mca_coll_earlybird_req_t *req;
    size_t root_shadow_size;
    int size;

    ompi_comm_size(comm, &size);

    /* Validate contiguous datatypes */
    if (!mca_coll_earlybird_is_contiguous(sendtype) || !mca_coll_earlybird_is_contiguous(recvtype)) {
        return OMPI_ERROR;
    }

    /* The root needs a shadow buffer for everyone's contribution */
    root_shadow_size = (size_t)size * recvcount * ompi_datatype_get_size(recvtype);

    req = OBJ_NEW(mca_coll_earlybird_req_t);
    if (!req) return OMPI_ERROR;

    ompi_request_init(&req->super, true);
    req->super.req_type = OMPI_REQUEST_COLL;

    req->sbuf = sendbuf;
    req->scount = sendcount;
    req->sdtype = sendtype;
    req->comm = comm;
    req->user_recvbuf = recvbuf;
    req->root = root;

    /* 
     * All ranks must create a window for MPI_Win_fence to work.
     * Root gets the large buffer; others get a minimal buffer as they are sources.
     */
    size_t my_shadow_size = (root == 0) ? root_shadow_size : 1; // Simplified for non-root
    // Correction: We need to know if 'this' rank is the root.
    int rank;
    ompi_comm_rank(comm, &rank);
    size_t actual_size = (rank == root) ? root_shadow_size : 1;

    if (mca_coll_earlybird_setup_window(req, actual_size, comm) != OMPI_SUCCESS) {
        OBJ_RELEASE(req);
        return OMPI_ERROR;
    }
    
    /* Store the total size the root is expecting so non-roots can compute offsets if needed, 
       though for Gather, the target is always the root's window. */
    req->shadow_buf_size = actual_size;

    OPAL_OUTPUT_VERBOSE((10, ompi_coll_base_framework.framework_output,
                        "EarlyBird: Gather init - rank %d, root %d, shadow_buf allocated (%zu bytes)",
                        rank, root, actual_size));

    *request = &req->super;
    return OMPI_SUCCESS;
}

/*
 * Gather Direct RMA Operation (Start)
 */
int mca_coll_earlybird_gather_start(size_t count, struct ompi_request_t **requests)
{
    for (size_t i = 0; i < count; i++) {
        mca_coll_earlybird_req_t *req = (mca_coll_earlybird_req_t *)requests[i];
        if (!req || req->super.req_type != OMPI_REQUEST_COLL) continue;

        int rank;
        ompi_comm_rank(req->comm, &rank);

        size_t send_bytes = req->scount * ompi_datatype_get_size(req->sdtype);
        size_t block_bytes = req->scount * ompi_datatype_get_size(req->sdtype);

        if (rank != req->root) {
            /* Non-root: Put contribution into root's shadow buffer */
            MPI_Put(req->sbuf, send_bytes, MPI_BYTE, req->root, rank * block_bytes, 
                    send_bytes, MPI_BYTE, req->win);
        } else {
            /* Root: Local copy of its own contribution into its shadow buffer */
            memcpy((char *)req->shadow_buf + (rank * block_bytes), 
                   req->sbuf, send_bytes);
        }
    }
    return OMPI_SUCCESS;
}

/*
 * Gather Direct RMA Wait
 */
void mca_coll_earlybird_gather_wait(ompi_request_t *req_base)
{
    mca_coll_earlybird_req_t *req = (mca_coll_earlybird_req_t *)req_base;
    int rank;
    ompi_comm_rank(req->comm, &rank);
    
    /* 1. Ensure all RMA puts are complete and visible at root */
    MPI_Win_fence(0, req->win);
    
    /* 2. Root copies the aggregated shadow buffer to user recvbuf */
    if (rank == req->root) {
        memcpy(req->user_recvbuf, req->shadow_buf, req->shadow_buf_size);
    }
    
    /* 3. Mandatory final fence to restore window consistency for reuse */
    MPI_Win_fence(0, req->win);
}

/*
 * Gather Direct RMA Free
 */
void mca_coll_earlybird_gather_free(ompi_request_t *req_base)
{
    mca_coll_earlybird_req_t *req = (mca_coll_earlybird_req_t *)req_base;
    
    mca_coll_earlybird_cleanup_window(req);
    OBJ_RELEASE(req);
}
