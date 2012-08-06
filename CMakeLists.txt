cmake_minimum_required (VERSION 2.6)
project (libseqev C)

set (LIB_VERSION_MAJOR "4")
set (LIB_VERSION_MINOR "13")
set (LIB_VERSION_PATCH "1")

install (FILES "${PROJECT_BINARY_DIR}/libseqev.a" DESTINATION lib)
install (FILES sequencer.h eventcounter.h seqev.h DESTINATION include)
install (FILES Sequencer.3 sequencer.3 Eventcounter.3 eventcounter.3 DESTINATION share/man/man3)
install (SCRIPT "${PROJECT_SOURCE_DIR}/lnmanpages.txt")

ADD_LIBRARY( seqev STATIC
   sequencer.c
   eventcounter.c)

# build a CPack driven installer package
# generall:
include (InstallRequiredSystemLibraries)
set (CPACK_PACKAGE_NAME "sysprog-libse")
set (CPACK_PACKAGE_CONTACT 
   "OSUE TEAM <osue-team@vmars.tuwien.ac.at>")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY 
   "Semaphor library for OSUE @ TU-WIEN")
set (CPACK_PACKAGE_VERSION_MAJOR "${LIB_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${LIB_VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${LIB_VERSION_PATCH}")

# DEB specific

# RPM specific
set (CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")

include (CPack)
