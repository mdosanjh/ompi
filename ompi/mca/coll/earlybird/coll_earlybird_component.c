/*
 * Copyright (c) 2026 Sandia National Laboratories. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

static int mca_coll_earlybird_open(void)
{
    return OMPI_SUCCESS;
}

static int mca_coll_earlybird_close(void)
{
    return OMPI_SUCCESS;
}

static int mca_coll_earlybird_register(void)
{
    mca_coll_earlybird_component_t *cm = &mca_coll_earlybird_component;
    mca_base_component_t *c = &cm->super.collm_version;

    mca_base_component_var_register(c, "priority", "Priority of the earlybird coll component",
                                    MCA_BASE_VAR_TYPE_INT, NULL, 0, MCA_BASE_VAR_FLAG_SETTABLE,
                                    OPAL_INFO_LVL_9, MCA_BASE_VAR_SCOPE_ALL, &cm->priority);

    mca_base_component_var_register(c, "verbose", "Verbose level of the earlybird coll component",
                                    MCA_BASE_VAR_TYPE_INT, NULL, 0, MCA_BASE_VAR_FLAG_SETTABLE,
                                    OPAL_INFO_LVL_9, MCA_BASE_VAR_SCOPE_ALL, &cm->verbose);

    mca_base_component_var_register(c, "enable", "[0|1] Enable/Disable the earlybird coll component",
                                    MCA_BASE_VAR_TYPE_INT, NULL, 0, MCA_BASE_VAR_FLAG_SETTABLE,
                                    OPAL_INFO_LVL_9, MCA_BASE_VAR_SCOPE_ALL, &cm->enable);

    return OMPI_SUCCESS;
}

mca_coll_earlybird_component_t mca_coll_earlybird_component = {
    {
        .collm_version = {
            MCA_COLL_BASE_VERSION_3_0_0,
            .mca_component_name = "earlybird",
            MCA_BASE_MAKE_VERSION(component, OMPI_MAJOR_VERSION, OMPI_MINOR_VERSION, OMPI_RELEASE_VERSION),
            .mca_open_component = mca_coll_earlybird_open,
            .mca_close_component = mca_coll_earlybird_close,
            .mca_register_component_params = mca_coll_earlybird_register,
            .mca_query_component = NULL,
        },
        .collm_data = {
            MCA_BASE_METADATA_PARAM_NONE
        },
        .collm_init_query = mca_coll_earlybird_init_query,
        .collm_comm_query = mca_coll_earlybird_comm_query,
    },
    0, /* default priority - low to be optional */
    0, /* verbose */
    0, /* enable */
};
MCA_BASE_COMPONENT_INIT(ompi, coll, earlybird)

int mca_coll_earlybird_init_query(bool enable_progress_threads, bool enable_mpi_threads)
{
    return OMPI_SUCCESS;
}
