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
 * Allgather Direct RMA Init
 */
int mca_coll_earlybird_allgather_init(const void *sbuf, size_t scount, struct ompi_datatype_t *sdtype,
                                      void *rbuf, size_t rcount, struct ompi_datatype_t *rdtype,
                                      struct ompi_communicator_t *comm, struct ompi_info_t *info,
                                      ompi_request_t **request, struct mca_coll_base_module_t *module)
{
    mca_coll_earlybird_req_t *req;
    size_t total_bytes;
    int size;

    ompi_comm_size(comm, &size);

    /* Validate contiguous datatypes */
    if (!mca_coll_earlybird_is_contiguous(sdtype) || !mca_coll_earlybird_is_contiguous(rdtype)) {
        return OMPI_ERROR; 
    }

    /* Total receive size: size * rcount * size_of_type */
    total_bytes = (size_t)size * rcount * ompi_datatype_get_size(rdtype);

    req = OBJ_NEW(mca_coll_earlybird_req_t);
    if (!req) return OMPI_ERROR;

    ompi_request_init(&req->super, true);
    req->super.req_type = OMPI_REQUEST_COLL;

    req->sbuf = sbuf;
    req->scount = scount;
    req->sdtype = sdtype;
    req->comm = comm;
    req->user_recvbuf = rbuf;

    /* Setup shadow buffer and MPI Window */
    if (mca_coll_earlybird_setup_window(req, total_bytes, comm) != OMPI_SUCCESS) {
        OBJ_RELEASE(req);
        return OMPI_ERROR;
    }

    OPAL_OUTPUT_VERBOSE((10, ompi_coll_base_framework.framework_output,
                        "EarlyBird: Allgather init - shadow_buf allocated (%zu bytes) and window created",
                        total_bytes));

    *request = &req->super;
    return OMPI_SUCCESS;
}

/*
 * Allgather Direct RMA Operation (Start)
 */
int mca_coll_earlybird_allgather_start(size_t count, struct ompi_request_t **requests)
{
    for (size_t i = 0; i < count; i++) {
        mca_coll_earlybird_req_t *req = (mca_coll_earlybird_req_t *)requests[i];
        if (!req || req->super.req_type != OMPI_REQUEST_COLL) continue;

        int rank, size;
        ompi_comm_rank(req->comm, &rank);
        ompi_comm_size(req->comm, &size);

        size_t element_size = ompi_datatype_get_size(req->sdtype);
        size_t send_bytes = req->scount * element_size;
        
        /* RMA Put contribution to the correct slot in every rank's shadow buffer */
        for (int j = 0; j < size; j++) {
            MPI_Put(req->sbuf, send_bytes, MPI_BYTE, j, rank * ompi_datatype_get_size(req->sdtype), 
                    send_bytes, MPI_BYTE, req->win);
        }
    }
    return OMPI_SUCCESS;
}

/*
 * Allgather Direct RMA Wait
 */
void mca_coll_earlybird_allgather_wait(ompi_request_t *req_base)
{
    mca_coll_earlybird_req_t *req = (mca_coll_earlybird_req_t *)req_base;
    
    /* 1. Ensure all RMA puts are complete and visible */
    MPI_Win_fence(0, req->win);
    
    /* 2. Copy from shadow buffer to user buffer */
    memcpy(req->user_recvbuf, req->shadow_buf, req->shadow_buf_size);
    
    /* 3. Mandatory final fence to restore window consistency for reuse */
    MPI_Win_fence(0, req->win);
}

/*
 * Allgather Direct RMA Free
 */
void mca_coll_earlybird_allgather_free(ompi_request_t *req_base)
{
    mca_coll_earlybird_req_t *req = (mca_coll_earlybird_req_t *)req_base;
    
    mca_coll_earlybird_cleanup_window(req);
    OBJ_RELEASE(req);
}
