## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = -I../include

noinst_libdir = ${exec_prefix}/lib/snort_dynamicpreprocessor

noinst_lib_LTLIBRARIES = lib_sfdynamic_preprocessor_example.la

lib_sfdynamic_preprocessor_example_la_LDFLAGS = -export-dynamic
lib_sfdynamic_preprocessor_example_la_LDADD = -lkyototycoon -lkyotocabinet -lstdc++ kyoto.lo

BUILT_SOURCES = \
sf_dynamic_preproc_lib.c  \
sfPolicyUserData.c

nodist_lib_sfdynamic_preprocessor_example_la_SOURCES = \
sf_dynamic_preproc_lib.c \
sfPolicyUserData.c \
spp_example.c \
sf_preproc_info.h

EXTRA_DIST = \
spp_example.cpp \
sf_preproc_info.h

sf_dynamic_preproc_lib.c: ../include/sf_dynamic_preproc_lib.c
	cp $? $@

sfPolicyUserData.c: ../include/sfPolicyUserData.c
	cp $? $@

clean-local:
	rm -f sf_dynamic_preproc_lib.c sfPolicyUserData.c

