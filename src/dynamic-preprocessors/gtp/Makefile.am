## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = -I../include -I${srcdir}/../libs -I$(srcdir)/includes

libdir = ${exec_prefix}/lib/snort_dynamicpreprocessor

lib_LTLIBRARIES = libsf_gtp_preproc.la

libsf_gtp_preproc_la_LDFLAGS = -shared -export-dynamic -module @XCCFLAGS@
if SO_WITH_STATIC_LIB
libsf_gtp_preproc_la_LIBADD = ../libsf_dynamic_preproc.la
else
nodist_libsf_gtp_preproc_la_SOURCES = \
../include/sf_dynamic_preproc_lib.c \
../include/sfPolicyUserData.c
endif

libsf_gtp_preproc_la_SOURCES = \
spp_gtp.c \
spp_gtp.h \
gtp_config.c \
gtp_config.h \
gtp_parser.c \
gtp_parser.h \
gtp_roptions.c \
gtp_roptions.h \
gtp_debug.h

EXTRA_DIST = \
sf_gtp.dsp


all-local: $(LTLIBRARIES)
	$(MAKE) DESTDIR=`pwd`/../build install-libLTLIBRARIES
