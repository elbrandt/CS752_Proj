add_executable (daxpy
        daxpy.cpp
        )

target_include_directories(daxpy
        PUBLIC
            ${GEM5_INC_DIR}
        )

target_link_libraries(daxpy
        ${M5_OBJS}
        )


message("GEM5_LIB_ROOT: ${GEM5_LIB_ROOT}")
message("GEM5_INC_DIR : ${GEM5_INC_DIR}")
message("M5_OBJS      : ${M5_OBJS}")
