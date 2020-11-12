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
