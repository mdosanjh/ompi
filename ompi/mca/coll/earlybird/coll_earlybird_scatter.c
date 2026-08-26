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
 * Scatter Direct RMA Init
 */
int mca_coll_earlybird_scatter_init(const void *sendbuf, size_t sendcount, struct ompi_datatype_t *sendtype,
                                   void *recvbuf, size_t recvcount, struct ompi_datatype_t *recvtype,
                                   int root, struct ompi_communicator_t *comm, struct ompi_info_t *info,
                                   ompi_request_t **request, struct mca_coll_base_module_t *module)
{
    mca_coll_earlybird_req_t *req;
    size_t element_size;
    int size, rank;

    ompi_comm_size(comm, &size);
    ompi_comm_rank(comm, &rank);

    /* Validate contiguous datatypes */
    if (!mca_coll_earlybird_is_contiguous(sendtype) || !mca_coll_earlybird_is_contiguous(recvtype)) {
        return OMPI_ERROR;
    }

    element_size = recvcount * ompi_datatype_get_size(recvtype);

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
     * Root needs a shadow buffer for all outgoing blocks (to facilitate local copy and RMA)
     * Non-roots need a shadow buffer for their own incoming block.
     */
    size_t actual_shadow_size = (rank == root) ? (size_t)size * element_size : element_size;

    if (mca_coll_earlybird_setup_window(req, actual_shadow_size, comm) != OMPI_SUCCESS) {
        OBJ_RELEASE(req);
        return OMPI_ERROR;
    }

    OPAL_OUTPUT_VERBOSE((10, ompi_coll_base_framework.framework_output,
                        "EarlyBird: Scatter init - rank %d, root %d, shadow_buf allocated (%zu bytes)",
                        rank, root, actual_shadow_size));

    *request = &req->super;
    return OMPI_SUCCESS;
}

/*
 * Scatter Direct RMA Operation (Start)
 */
int mca_coll_earlybird_scatter_start(size_t count, struct ompi_request_t **requests)
{
    for (size_t i = 0; i < count; i++) {
        mca_coll_earlybird_req_t *req = (mca_coll_earlybird_req_t *)requests[i];
        if (!req || req->super.req_type != OMPI_REQUEST_COLL) continue;

        int rank;
        ompi_comm_rank(req->comm, &rank);

        if (rank == req->root) {
            int size;
            ompi_comm_size(req->comm, &size);
            size_t block_bytes = req->scount * ompi_datatype_get_size(req->sdtype);

            /* Root puts each peer's block into their shadow buffer at offset 0 */
            for (int j = 0; j < size; j++) {
                if (j == rank) {
                    /* Local copy for root's own block into its shadow buffer at its specific offset */
                    memcpy((char *)req->shadow_buf + (rank * block_bytes), 
                           (char *)req->sbuf + (rank * block_bytes), block_bytes);
                } else {
                    MPI_Put((char *)req->sbuf + (j * block_bytes), block_bytes, MPI_BYTE, j, 0, 
                            block_bytes, MPI_BYTE, req->win);
                }
            }
        }
        /* Non-root ranks are passive targets */
    }
    return OMPI_SUCCESS;
}

/*
 * Scatter Direct RMA Wait
 */
void mca_coll_earlybird_scatter_wait(ompi_request_t *req_base)
{
    mca_coll_earlybird_req_t *req = (mca_coll_earlybird_req_t *)req_base;
    int rank;
    ompi_comm_rank(req->comm, &rank);
    
    /* 1. Ensure all RMA puts from root are complete and visible */
    MPI_Win_fence(0, req->win);
    
    /* 2. Copy the received block from shadow buffer to user recvbuf */
    size_t block_bytes = req->scount * ompi_datatype_get_size(req->sdtype);
    void *src = (rank == req->root) ? ((char *)req->shadow_buf + (rank * block_bytes)) : req->shadow_buf;
    
    memcpy(req->user_recvbuf, src, block_bytes);
    
    /* 3. Mandatory final fence to restore window consistency for reuse */
    MPI_Win_fence(0, req->win);
}

/*
 * Scatter Direct RMA Free
 */
void mca_coll_earlybird_scatter_free(ompi_request_t *req_base)
{
    mca_coll_earlybird_req_t *req = (mca_coll_earlybird_req_t *)req_base;
    
    mca_coll_earlybird_cleanup_window(req);
    OBJ_RELEASE(req);
}
