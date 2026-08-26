/*
 * Copyright (c) 2026 Sandia National Laboratories. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include "coll.h"
#include "ompi/mca/coll/base/base.h"
#include "ompi/datatype/datatype.h"

BEGIN_C_DECLS

typedef struct mca_coll_earlybird_component_t {
    mca_coll_base_component_t super;
    int priority;
    int verbose;
    int enable;
} mca_coll_earlybird_component_t;

typedef struct mca_coll_earlybird_module_t {
    mca_coll_base_module_t super;
    ompi_communicator_t *comm;
} mca_coll_earlybird_module_t;

typedef struct mca_coll_earlybird_req_t {
    ompi_request_t super;
    MPI_Win win;
    void *shadow_buf;           /* Staging buffer for RMA */
    void *user_recvbuf;         /* Final destination */
    size_t shadow_buf_size;     /* Total size of shadow buffer */

    const void *sbuf;           /* Source buffer */
    size_t scount;              /* Count of elements */
    struct ompi_datatype_t *sdtype;
    struct ompi_communicator_t *comm;
    int root;                   /* Root rank for rooted collectives */
} mca_coll_earlybird_req_t;

/* Prototypes for the component functions */
int mca_coll_earlybird_init_query(bool enable_progress_threads, bool enable_mpi_threads);
mca_coll_base_module_t *mca_coll_earlybird_comm_query(struct ompi_communicator_t *comm, int *priority);

/* Allgather */
int mca_coll_earlybird_allgather_init(const void *sbuf, size_t scount, struct ompi_datatype_t *sdtype,
                                      void *rbuf, size_t rcount, struct ompi_datatype_t *rdtype,
                                      struct ompi_communicator_t *comm, struct ompi_info_t *info,
                                      ompi_request_t **request, struct mca_coll_base_module_t *module);
int mca_coll_earlybird_allgather_start(size_t count, struct ompi_request_t **requests);
void mca_coll_earlybird_allgather_wait(ompi_request_t *req);
void mca_coll_earlybird_allgather_free(ompi_request_t *req);

/* Alltoall */
int mca_coll_earlybird_alltoall_init(const void *sbuf, size_t sendcount, struct ompi_datatype_t *sendtype,
                                    void *rbuf, size_t recvcount, struct ompi_datatype_t *recvtype,
                                    struct ompi_communicator_t *comm, struct ompi_info_t *info,
                                    ompi_request_t **request, struct mca_coll_base_module_t *module);
int mca_coll_earlybird_alltoall_start(size_t count, struct ompi_request_t **requests);
void mca_coll_earlybird_alltoall_wait(ompi_request_t *req);
void mca_coll_earlybird_alltoall_free(ompi_request_t *req);

/* Broadcast */
int mca_coll_earlybird_bcast_init(void *buffer, size_t count, struct ompi_datatype_t *datatype,
                                 int root, struct ompi_communicator_t *comm, struct ompi_info_t *info,
                                 ompi_request_t **request, struct mca_coll_base_module_t *module);
int mca_coll_earlybird_bcast_start(size_t count, struct ompi_request_t **requests);
void mca_coll_earlybird_bcast_wait(ompi_request_t *req);
void mca_coll_earlybird_bcast_free(ompi_request_t *req);

/* Gather */
int mca_coll_earlybird_gather_init(const void *sendbuf, size_t sendcount, struct ompi_datatype_t *sendtype,
                                  void *recvbuf, size_t recvcount, struct ompi_datatype_t *recvtype,
                                  int root, struct ompi_communicator_t *comm, struct ompi_info_t *info,
                                  ompi_request_t **request, struct mca_coll_base_module_t *module);
int mca_coll_earlybird_gather_start(size_t count, struct ompi_request_t **requests);
void mca_coll_earlybird_gather_wait(ompi_request_t *req);
void mca_coll_earlybird_gather_free(ompi_request_t *req);

/* Scatter */
int mca_coll_earlybird_scatter_init(const void *sendbuf, size_t sendcount, struct ompi_datatype_t *sendtype,
                                   void *recvbuf, size_t recvcount, struct ompi_datatype_t *recvtype,
                                   int root, struct ompi_communicator_t *comm, struct ompi_info_t *info,
                                   ompi_request_t **request, struct mca_coll_base_module_t *module);
int mca_coll_earlybird_scatter_start(size_t count, struct ompi_request_t **requests);
void mca_coll_earlybird_scatter_wait(ompi_request_t *req);
void mca_coll_earlybird_scatter_free(ompi_request_t *req);

END_C_DECLS

#endif /* OMPI_MCA_COLL_EARLYBIRD_COLL_H */
