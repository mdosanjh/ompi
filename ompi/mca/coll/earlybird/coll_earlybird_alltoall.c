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
 * Alltoall Direct RMA Init
 */
int mca_coll_earlybird_alltoall_init(const void *sbuf, size_t sendcount, struct ompi_datatype_t *sendtype,
                                    void *rbuf, size_t recvcount, struct ompi_datatype_t *recvtype,
                                    struct ompi_communicator_t *comm, struct ompi_info_t *info,
                                    ompi_request_t **request, struct mca_coll_base_module_t *module)
{
    mca_coll_earlybird_req_t *req;
    size_t total_bytes;
    int size;

    ompi_comm_size(comm, &size);

    /* Validate contiguous datatypes */
    if (!mca_coll_earlybird_is_contiguous(sendtype) || !mca_coll_earlybird_is_contiguous(recvtype)) {
        return OMPI_ERROR;
    }

    /* Total receive size: size * recvcount * size_of_type */
    total_bytes = (size_t)size * recvcount * ompi_datatype_get_size(recvtype);

    req = OBJ_NEW(mca_coll_earlybird_req_t);
    if (!req) return OMPI_ERROR;

    ompi_request_init(&req->super, true);
    req->super.req_type = OMPI_REQUEST_COLL;

    req->sbuf = sbuf;
    req->scount = sendcount;
    req->sdtype = sendtype;
    req->comm = comm;
    req->user_recvbuf = rbuf;

    /* Setup shadow buffer and MPI Window */
    if (mca_coll_earlybird_setup_window(req, total_bytes, comm) != OMPI_SUCCESS) {
        OBJ_RELEASE(req);
        return OMPI_ERROR;
    }

    OPAL_OUTPUT_VERBOSE((10, ompi_coll_base_framework.framework_output,
                        "EarlyBird: Alltoall init - shadow_buf allocated (%zu bytes) and window created",
                        total_bytes));

    *request = &req->super;
    return OMPI_SUCCESS;
}

/*
 * Alltoall Direct RMA Operation (Start)
 */
int mca_coll_earlybird_alltoall_start(size_t count, struct ompi_request_t **requests)
{
    for (size_t i = 0; i < count; i++) {
        mca_coll_earlybird_req_t *req = (mca_coll_earlybird_req_t *)requests[i];
        if (!req || req->super.req_type != OMPI_REQUEST_COLL) continue;

        int rank, size;
        ompi_comm_rank(req->comm, &rank);
        ompi_comm_size(req->comm, &size);

        size_t send_block_bytes = req->scount * ompi_datatype_get_size(req->sdtype);
        size_t recv_block_bytes = send_block_bytes; // Blocks must be sized by count * type_size

        /* Pairwise RMA Put of blocks to all peers' shadow buffers */
        for (int j = 0; j < size; j++) {
            if (j == rank) {
                /* Local copy for self-contribution */
                memcpy((char *)req->shadow_buf + (rank * recv_block_bytes), 
                       (char *)req->sbuf + (rank * send_block_bytes), send_block_bytes);
            } else {
                /* Put block j into peer j's shadow buffer at rank's slot */
                MPI_Put((char *)req->sbuf + (j * send_block_bytes), send_block_bytes, MPI_BYTE, j, 
                        rank * recv_block_bytes, send_block_bytes, MPI_BYTE, req->win);
            }
        }
    }
    return OMPI_SUCCESS;
}

/*
 * Alltoall Direct RMA Wait
 */
void mca_coll_earlybird_alltoall_wait(ompi_request_t *req_base)
{
    mca_coll_earlybird_req_t *req = (mca_coll_earlybird_req_t *)req_base;
    
    /* 1. Ensure all pairwise RMA puts are complete and visible */
    MPI_Win_fence(0, req->win);
    
    /* 2. Copy fully assembled shadow buffer to user recvbuf */
    memcpy(req->user_recvbuf, req->shadow_buf, req->shadow_buf_size);
    
    /* 3. Mandatory final fence to restore window consistency for reuse */
    MPI_Win_fence(0, req->win);
}

/*
 * Alltoall Direct RMA Free
 */
void mca_coll_earlybird_alltoall_free(ompi_request_t *req_base)
{
    mca_coll_earlybird_req_t *req = (mca_coll_earlybird_req_t *)req_base;
    
    mca_coll_earlybird_cleanup_window(req);
    OBJ_RELEASE(req);
}
