cmake_minimum_required (VERSION 2.8)
project (libseqev C)

set (LIB_VERSION_MAJOR "4")
set (LIB_VERSION_MINOR "14")
set (LIB_VERSION_PATCH "1")

set (COMMON_FLAGS "-std=c99 -Wall -pedantic -D_XOPEN_SOURCE=500")
if (CMAKE_BUILD_TYPE STREQUAL "debug")
   set (DEBUG_FLAGS "-O0 -g")
else()
   set (CMAKE_BUILD_TYPE "release")
endif()
message ("-- Building target: ${CMAKE_BUILD_TYPE}")

set (CMAKE_C_FLAGS_RELEASE "${COMMON_FLAGS}")
set (CMAKE_C_FLAGS_DEBUG "${COMMON_FLAGS} ${DEBUG_FLAGS}")

include_directories (${CMAKE_SOURCE_DIR})

install (FILES "${PROJECT_BINARY_DIR}/libseqev.a" DESTINATION lib)
install (FILES sequencer.h eventcounter.h seqev.h DESTINATION include)
install (FILES Sequencer.3 sequencer.3 Eventcounter.3 eventcounter.3 DESTINATION share/man/man3)
install (SCRIPT "${PROJECT_SOURCE_DIR}/lnmanpages.txt")

ADD_LIBRARY( seqev STATIC
   sequencer.c
   eventcounter.c
   seqev.c)

# enable testing in debug mode
if (CMAKE_BUILD_TYPE STREQUAL "debug")
   ENABLE_TESTING()
   
   # SE
   add_test(seqev ${CMAKE_CURRENT_BINARY_DIR}/alternate_seqev)
   set_tests_properties (seqev
      PROPERTIES PASS_REGULAR_EXPRESSION "01Xp10p21p12p23p14p25p16p27p18p29" TIMEOUT 5)
   
   add_executable(alternate_seqev tests/alternate_seqev.c tests/common.c)
   target_link_libraries(alternate_seqev seqev)
endif() # enable testing in debug mode

# build a CPack driven installer package
# generall:
include (InstallRequiredSystemLibraries)
set (CPACK_PACKAGE_NAME "osue-${CMAKE_PROJECT_NAME}")
set (CPACK_PACKAGE_CONTACT 
   "OSUE TEAM <osue-team@vmars.tuwien.ac.at>")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY 
   "Sequencer/Eventcounter library for OSUE @ TU-WIEN")
set (CPACK_PACKAGE_VERSION_MAJOR "${LIB_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${LIB_VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${LIB_VERSION_PATCH}")

include (CPack)
