add_executable (MembranePinch1
        main.cpp
        )

find_package(Eigen3)

# if CMake cannot use find_package for Eigen, use the explicit include path instead
# set(EIGEN3_INCLUDE_DIRS /usr/include/eigen3/)

target_include_directories(MembranePinch1
        PUBLIC
            ${EIGEN3_INCLUDE_DIRS}
        )