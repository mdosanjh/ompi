/*
 * Copyright (c) 2026 Sandia National Laboratories. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

BEGIN_C_DECLS

/* Returns true if the datatype is contiguous, false otherwise */
bool mca_coll_earlybird_is_contiguous(struct ompi_datatype_t *type);

/* Helper to allocate shadow buffer and create MPI Window */
int mca_coll_earlybird_setup_window(mca_coll_earlybird_req_t *req, size_t size, struct ompi_communicator_t *comm);

/* Helper to free window and shadow buffer */
void mca_coll_earlybird_cleanup_window(mca_coll_earlybird_req_t *req);

END_C_DECLS

#endif /* OMPI_MCA_COLL_EARLYBIRD_UTILS_H */
