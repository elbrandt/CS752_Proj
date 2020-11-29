cmake_minimum_required(VERSION 2.8)
project("povray")

file(GLOB povray_SRC
    "*.h"
    "*.cpp"
)

add_compile_options(-fpermissive -Wno-write-strings -Wno-multichar -Wno-aggressive-loop-optimizations -Wno-unused-result -Wno-terminate -Wno-stringop-overflow -Wno-format-zero-length)
add_executable(${PROJECT_NAME} ${povray_SRC})

# make a symlink from the data dir to the output dir
set (data_source "${CMAKE_CURRENT_SOURCE_DIR}/../data")
set (data_destination "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}_data")
add_custom_command(
 TARGET ${PROJECT_NAME} POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E create_symlink ${data_source} ${data_destination} 
 DEPENDS ${destination}
 COMMENT "symbolic link data folder from ${data_source} => ${data_destination}"
)
