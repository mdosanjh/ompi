/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2006-2007 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2007 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2006 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2015      Los Alamos National Security, LLC. All rights
 *                         reserved.
 * Copyright (c) 2011-2021 Sandia National Laboratories. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include "ompi_config.h"

#include "ompi/communicator/communicator.h"
#include "ompi/mca/part/base/part_base_prequest.h"
#include "ompi/mca/part/base/base.h"

#include "part_persist.h"
#include "part_persist_sendreq.h"
#include "part_persist_recvreq.h"

ompi_part_persist_t ompi_part_persist = {
    {
        mca_part_persist_progress,
        mca_part_persist_precv_init,
        mca_part_persist_psend_init,
        mca_part_persist_start,
        mca_part_persist_pready,
        mca_part_persist_parrived, 
    }
};


OBJ_CLASS_INSTANCE(mca_part_persist_list_t,
                   opal_list_item_t,
                   NULL,
                   NULL);

