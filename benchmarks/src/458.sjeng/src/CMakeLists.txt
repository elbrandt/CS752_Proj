cmake_minimum_required(VERSION 2.8)
project("sjeng")

file(GLOB ${PROJECT_NAME}_SRC
    "*.h"
    "*.c"
)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRC})

# make a symlink from the data dir to the output dir
set (data_source "${CMAKE_CURRENT_SOURCE_DIR}/../data")
set (data_destination "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}_data")
add_custom_command(
 TARGET ${PROJECT_NAME} POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E create_symlink ${data_source} ${data_destination} 
 DEPENDS ${destination}
 COMMENT "symbolic link data folder from ${data_source} => ${data_destination}"
)
