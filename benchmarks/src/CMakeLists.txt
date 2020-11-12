cmake_minimum_required (VERSION 2.8.11)
set (CMAKE_CXX_STANDARD 11)

project (SimBenchmarks)

set (CMAKE_CXX_FLAGS "-Wno-deprecated ${CMAKE_CXX_FLAGS}")

set(GEM5_INC_DIR $ENV{GEM5_ROOT}/include/gem5)
set(GEM5_LIB_ROOT $ENV{GEM5_ROOT}/util/m5/build/x86)

set(M5_OBJS
        ${GEM5_LIB_ROOT}/abi/x86/m5op.o
        ${GEM5_LIB_ROOT}/m5_mmap.o
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../../bin)

add_subdirectory(daxpy)
add_subdirectory(MembranePinch1)