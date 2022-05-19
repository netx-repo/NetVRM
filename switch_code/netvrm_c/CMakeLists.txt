cmake_minimum_required(VERSION 3.0)

project(CP)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set (CMAKE_CXX_COMPILER "g++")
set (CMAKE_CXX_FLAGS "--std=gnu++14 -lpthread ${CMAKE_CXX_FLAGS}")
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED True)

set (PROGNAME "netvrm_wan")
set (ARCH "tofino")

set (TOFINO_INSTALL
    $ENV{SDE_INSTALL}
    )

set (PD_LIB_PATH
    "${TOFINO_INSTALL}/lib/${ARCH}pd/${PROGNAME}"
    )


set (SRCs
    src/main.cc
    src/application.cc
    src/options.cc
    src/tests/test.cc
    src/tests/equal-active-test.cc
    src/tests/equal-all-test.cc
    src/tests/netvrm-test.cc
    )

set (TOFINO_LIBS
    bf_switchd_lib
    bfsys
    bfutils
    thrift
    tofinopdfixed_thrift
    pd
    pdthrift
    driver
    )

set (TOFINO_LIBS "${TOFINO_LIBS};${CMAKE_THREAD_LIBS_INIT};")


link_directories(${TOFINO_INSTALL}/lib)
link_directories(${PD_LIB_PATH})

add_executable(CP ${SRCs})

message(STATUS "${PD_LIB_PATH}")

target_include_directories(CP PRIVATE ${TOFINO_INSTALL}/include)
target_link_libraries(CP PUBLIC ${TOFINO_LIBS})

configure_file(run.sh run.sh COPYONLY)
configure_file(set_bash set_bash COPYONLY)
