/*
 * Copyright (c) 2026 Sandia National Laboratories. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

bool mca_coll_earlybird_is_contiguous(struct ompi_datatype_t *type)
{
    if (!type) return false;
    return ompi_datatype_is_contiguous(type);
}

int mca_coll_earlybird_setup_window(mca_coll_earlybird_req_t *req, size_t size, struct ompi_communicator_t *comm)
{
    req->shadow_buf_size = size;
    req->shadow_buf = OPAL_MALLOC(size);
    if (!req->shadow_buf) {
        return OMPI_ERROR;
    }

    if (MPI_Win_create(req->shadow_buf, size, 1, MPI_INFO_NULL, comm, &req->win) != MPI_SUCCESS) {
        OPAL_FREE(req->shadow_buf);
        req->shadow_buf = NULL;
        return OMPI_ERROR;
    }

    /* Initialize window state */
    MPI_Win_fence(0, req->win);

    return OMPI_SUCCESS;
}

void mca_coll_earlybird_cleanup_window(mca_coll_earlybird_req_t *req)
{
    if (req->win != MPI_WIN_NULL) {
        MPI_Win_free(&req->win);
    }
    if (req->shadow_buf) {
        OPAL_FREE(req->shadow_buf);
        req->shadow_buf = NULL;
    }
}
