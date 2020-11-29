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


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../../bin)

message("CMAKE_RUNTIME_OUTPUT_DIRECTORY: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message("GEM5_LIB_ROOT: ${GEM5_LIB_ROOT}")
message("GEM5_INC_DIR : ${GEM5_INC_DIR}")
message("M5_OBJS      : ${M5_OBJS}")
message("Output dir   : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

add_subdirectory(daxpy)
add_subdirectory(MembranePinch1)
add_subdirectory(453.povray/src)
add_subdirectory(458.sjeng/src)

# copy all the run scripts to the bin dir
set (scripts_dir "${CMAKE_SOURCE_DIR}/../run_scripts")
file(GLOB run_scripts "${scripts_dir}/*.sh")

message("scripts_dir:" ${scripts_dir})
message("run_scripts:" ${run_scripts})
add_custom_target(run_scripts ALL DEPENDS ${run_scripts})
add_custom_command(
 OUTPUT ${run_scripts} POST_BUILD CONFIGURE_DEPENDS
 COMMAND ${CMAKE_COMMAND} -E copy ${run_scripts} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} 
 DEPENDS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
 COMMENT "Copy run script files from ${scripts_dir} to output directory"
)
