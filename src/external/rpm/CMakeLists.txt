cmake_minimum_required(VERSION 3.12.4)

include(ExternalProject)

project(librpm)

get_filename_component(SRC_FOLDER ${CMAKE_SOURCE_DIR}/../ ABSOLUTE)
include_directories(${SRC_FOLDER}/lua/install/include/)

set(LIBRPM_SRC ${CMAKE_CURRENT_SOURCE_DIR})
set(LIBRPM_OUT ${CMAKE_CURRENT_BINARY_DIR}/output)
set(LIBRPM_PUBLIC_INCLUDES ${LIBRPM_OUT}/include)
set(LIBRPM_STATIC_LIB ${LIBRPM_OUT}/librpm.a)


set(library_headers
  "${LIBRPM_SRC}/include/rpm/argv.h"
  "${LIBRPM_SRC}/include/rpm/rpmcrypto.h"
  "${LIBRPM_SRC}/include/rpm/rpmio.h"
  "${LIBRPM_SRC}/include/rpm/rpmurl.h"
  "${LIBRPM_SRC}/include/rpm/rpmmacro.h"
  "${LIBRPM_SRC}/include/rpm/rpmlog.h"
  "${LIBRPM_SRC}/include/rpm/rpmpgp.h"
  "${LIBRPM_SRC}/include/rpm/rpmsq.h"
  "${LIBRPM_SRC}/include/rpm/rpmstring.h"
  "${LIBRPM_SRC}/include/rpm/rpmstrpool.h"
  "${LIBRPM_SRC}/include/rpm/rpmsw.h"
  "${LIBRPM_SRC}/include/rpm/rpmfileutil.h"
  "${LIBRPM_SRC}/include/rpm/rpmutil.h"
  "${LIBRPM_SRC}/include/rpm/rpmkeyring.h"
  "${LIBRPM_SRC}/include/rpm/rpmbase64.h"
  "${LIBRPM_SRC}/include/rpm/rpmver.h"
  "${LIBRPM_SRC}/include/rpm/header.h"
  "${LIBRPM_SRC}/include/rpm/rpmdb.h"
  "${LIBRPM_SRC}/include/rpm/rpmcallback.h"
  "${LIBRPM_SRC}/include/rpm/rpmcli.h"
  "${LIBRPM_SRC}/include/rpm/rpmlib.h"
  "${LIBRPM_SRC}/include/rpm/rpmds.h"
  "${LIBRPM_SRC}/include/rpm/rpmfi.h"
  "${LIBRPM_SRC}/include/rpm/rpmfiles.h"
  "${LIBRPM_SRC}/include/rpm/rpmpol.h"
  "${LIBRPM_SRC}/include/rpm/rpmps.h"
  "${LIBRPM_SRC}/include/rpm/rpmprob.h"
  "${LIBRPM_SRC}/include/rpm/rpmtag.h"
  "${LIBRPM_SRC}/include/rpm/rpmtd.h"
  "${LIBRPM_SRC}/include/rpm/rpmte.h"
  "${LIBRPM_SRC}/include/rpm/rpmts.h"
  "${LIBRPM_SRC}/include/rpm/rpmtypes.h"
  "${LIBRPM_SRC}/include/rpm/rpmarchive.h"
  "${LIBRPM_SRC}/rpmio/lposix.h"
  "${LIBRPM_SRC}/rpmio/rpmlua.h"
  "${LIBRPM_SRC}/rpmio/rpmmacro_internal.h"
)

set(rpm_library_srcs
  "${LIBRPM_SRC}/lib/backend/bdb_ro.c"
  "${LIBRPM_SRC}/lib/backend/sqlite.c"
  "${LIBRPM_SRC}/lib/backend/dbi.c"
  "${LIBRPM_SRC}/lib/backend/dummydb.c"
  "${LIBRPM_SRC}/lib/backend/dbiset.c"
  "${LIBRPM_SRC}/lib/backend/ndb/glue.c"
  "${LIBRPM_SRC}/lib/backend/ndb/rpmidx.c"
  "${LIBRPM_SRC}/lib/backend/ndb/rpmpkg.c"
  "${LIBRPM_SRC}/lib/backend/ndb/rpmxdb.c"
  "${LIBRPM_SRC}/lib/header.c"
  "${LIBRPM_SRC}/lib/headerfmt.c"
  "${LIBRPM_SRC}/lib/headerutil.c"
  "${LIBRPM_SRC}/lib/rpmdb.c"
  "${LIBRPM_SRC}/lib/fprint.c"
  "${LIBRPM_SRC}/lib/tagname.c"
  "${LIBRPM_SRC}/lib/rpmtd.c"
  "${LIBRPM_SRC}/lib/cpio.c"
  "${LIBRPM_SRC}/lib/depends.c"
  "${LIBRPM_SRC}/lib/order.c"
  "${LIBRPM_SRC}/lib/formats.c"
  "${LIBRPM_SRC}/lib/tagexts.c"
  "${LIBRPM_SRC}/lib/fsm.c"
  "${LIBRPM_SRC}/lib/manifest.c"
  "${LIBRPM_SRC}/lib/package.c"
  "${LIBRPM_SRC}/lib/poptALL.c"
  "${LIBRPM_SRC}/lib/poptI.c"
  "${LIBRPM_SRC}/lib/poptQV.c"
  "${LIBRPM_SRC}/lib/psm.c"
  "${LIBRPM_SRC}/lib/query.c"
  "${LIBRPM_SRC}/lib/rpmal.c"
  "${LIBRPM_SRC}/lib/rpmchecksig.c"
  "${LIBRPM_SRC}/lib/rpmds.c"
  "${LIBRPM_SRC}/lib/rpmfi.c"
  "${LIBRPM_SRC}/lib/rpmgi.c"
  "${LIBRPM_SRC}/lib/rpminstall.c"
  "${LIBRPM_SRC}/lib/rpmlead.c"
  "${LIBRPM_SRC}/lib/rpmps.c"
  "${LIBRPM_SRC}/lib/rpmprob.c"
  "${LIBRPM_SRC}/lib/rpmrc.c"
  "${LIBRPM_SRC}/lib/rpmte.c"
  "${LIBRPM_SRC}/lib/rpmts.c"
  "${LIBRPM_SRC}/lib/rpmfs.c"
  "${LIBRPM_SRC}/lib/signature.c"
  "${LIBRPM_SRC}/lib/transaction.c"
  "${LIBRPM_SRC}/lib/verify.c"
  "${LIBRPM_SRC}/lib/rpmlock.c"
  "${LIBRPM_SRC}/lib/relocation.c"
  "${LIBRPM_SRC}/lib/rpmscript.c"
  "${LIBRPM_SRC}/lib/rpmchroot.c"
  "${LIBRPM_SRC}/lib/rpmplugins.c"
  "${LIBRPM_SRC}/lib/rpmug.c"
  "${LIBRPM_SRC}/lib/rpmtriggers.c"
  "${LIBRPM_SRC}/lib/rpmvs.c"
)

set(rpmio_library_srcs
  "${LIBRPM_SRC}/rpmio/argv.c"
  "${LIBRPM_SRC}/rpmio/base64.c"
  "${LIBRPM_SRC}/rpmio/digest.c"
  "${LIBRPM_SRC}/rpmio/expression.c"
  "${LIBRPM_SRC}/rpmio/lposix.c"
  "${LIBRPM_SRC}/rpmio/macro.c"
  "${LIBRPM_SRC}/rpmio/rgetopt.c"
  "${LIBRPM_SRC}/rpmio/rpmhook.c"
  "${LIBRPM_SRC}/rpmio/rpmio.c"
  "${LIBRPM_SRC}/rpmio/rpmlog.c"
  "${LIBRPM_SRC}/rpmio/rpmlua.c"
  "${LIBRPM_SRC}/rpmio/rpmmalloc.c"
  "${LIBRPM_SRC}/rpmio/rpmpgp.c"
  "${LIBRPM_SRC}/rpmio/rpmpgp_internal.c"
  "${LIBRPM_SRC}/rpmio/rpmsq.c"
  "${LIBRPM_SRC}/rpmio/rpmsw.c"
  "${LIBRPM_SRC}/rpmio/url.c"
  "${LIBRPM_SRC}/rpmio/rpmstring.c"
  "${LIBRPM_SRC}/rpmio/rpmfileutil.c"
  "${LIBRPM_SRC}/rpmio/rpmglob.c"
  "${LIBRPM_SRC}/rpmio/rpmkeyring.c"
  "${LIBRPM_SRC}/rpmio/rpmstrpool.c"
  "${LIBRPM_SRC}/rpmio/rpmver.c"
  "${LIBRPM_SRC}/rpmio/rpmvercmp.c"
  "${LIBRPM_SRC}/rpmio/digest_openssl.c"
)

add_library(rpm STATIC
  ${rpm_library_srcs}
  ${rpmio_library_srcs}
)

file(MAKE_DIRECTORY ${LIBRPM_PUBLIC_INCLUDES}/rpm)

add_custom_target(
        rpm_create_headers ALL
        COMMAND ${CMAKE_COMMAND} -E copy
                ${library_headers}
                ${LIBRPM_PUBLIC_INCLUDES}/rpm)

add_dependencies(rpm rpm_create_headers)

target_compile_definitions(rpm PRIVATE
  HAVE_CONFIG_H
  LIBRPMALIAS_EXECPATH="/usr/bin"
  LIBRPMALIAS_FILENAME="rpmpopt-4.16.1.2"
  LOCALEDIR="/usr/share/locale"
  LOCALSTATEDIR="/usr/var"
  PIC
  _REENTRANT
  RPMCONFIGDIR="/usr/lib/rpm"
  SYSCONFDIR="/usr/etc"
)

target_include_directories(rpm PRIVATE
  "${LIBRPM_SRC}"
  "${LIBRPM_PUBLIC_INCLUDES}"
)

set_target_properties(rpm PROPERTIES IMPORTED_LOCATION ${LIBRPM_STATIC_LIB})
set_target_properties(rpm PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${LIBRPM_PUBLIC_INCLUDES})
