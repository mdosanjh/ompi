# EarlyBird Collective Component

sources =         coll_earlybird.h         coll_earlybird_utils.h         coll_earlybird_component.c         coll_earlybird_module.c         coll_earlybird_allgather.c         coll_earlybird_alltoall.c         coll_earlybird_bcast.c         coll_earlybird_gather.c         coll_earlybird_scatter.c         coll_earlybird_utils.c

# Make the output library in this directory, and name it either
# mca_<type>_<name>.la (for DSO builds) or libmca_<type>_<name>.la
# (for static builds).

if MCA_BUILD_ompi_coll_earlybird_DSO
component_noinst =
component_install = mca_coll_earlybird.la
else
component_noinst = libmca_coll_earlybird.la
component_install =
endif

mcacomponentdir = $(ompilibdir)
mcacomponent_LTLIBRARIES = $(component_install)

mca_coll_earlybird_la_SOURCES = $(sources)
mca_coll_earlybird_la_LDFLAGS = -module -avoid-version
mca_coll_earlybird_la_LIBADD = $(top_builddir)/ompi/libopen_mpi.la

noinst_LTLIBRARIES = $(component_noinst)
libmca_coll_earlybird_la_SOURCES = $(sources)
libmca_coll_earlybird_la_LDFLAGS = -module -avoid-version
libmca_coll_earlybird_la_LIBADD = $(top_builddir)/ompi/libopen_mpi.la

# Header installation
if WANT_INSTALL_HEADERS
ompidir = $(ompiincludedir)/$(subdir)
nobase_ompi_HEADERS = coll_earlybird.h
endif
