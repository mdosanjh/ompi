/*
 * Copyright (c) 2026 Sandia National Laboratories. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

static void mca_coll_earlybird_module_construct(mca_coll_earlybird_module_t *module)
{
    module->comm = NULL;
}

static void mca_coll_earlybird_module_destruct(mca_coll_earlybird_module_t *module)
{
}

static int mca_coll_earlybird_module_enable(mca_coll_base_module_t *module,
                                           struct ompi_communicator_t *comm)
{
    return OMPI_SUCCESS;
}

static int mca_coll_earlybird_module_disable(mca_coll_base_module_t *module,
                                            struct ompi_communicator_t *comm)
{
    return OMPI_SUCCESS;
}

mca_coll_base_module_t *
mca_coll_earlybird_comm_query(struct ompi_communicator_t *comm, int *priority)
{
    mca_coll_earlybird_component_t *cm = &mca_coll_earlybird_component;
    mca_coll_earlybird_module_t *module;

    if (!cm->enable) {
        return NULL;
    }

    module = OBJ_NEW(mca_coll_earlybird_module_t);
    if (!module) {
        return NULL;
    }
    module->comm = comm;
    module->super.coll_module_enable = mca_coll_earlybird_module_enable;
    module->super.coll_module_disable = mca_coll_earlybird_module_disable;
    
    /* Wire the EarlyBird RMA Direct implementations */
    module->super.coll_allgather_init = mca_coll_earlybird_allgather_init;
    module->super.coll_alltoall_init  = mca_coll_earlybird_alltoall_init;
    module->super.coll_bcast_init     = mca_coll_earlybird_bcast_init;
    module->super.coll_gather_init    = mca_coll_earlybird_gather_init;
    module->super.coll_scatter_init   = mca_coll_earlybird_scatter_init;
       
    *priority = cm->priority;

    return &module->super;
}

OBJ_CLASS_INSTANCE(mca_coll_earlybird_module_t,
                   mca_coll_base_module_t,
                   mca_coll_earlybird_module_construct,
                   mca_coll_earlybird_module_destruct);
