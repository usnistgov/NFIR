
cmake_minimum_required(VERSION 3.10)

# If this error is encountered:
#  "CMake Error in /.../NFRLNFIR/build/CMakeFiles/CMakeTmp/CMakeLists.txt:
#   Target "cmTC_32aa2" requires the language dialect "CXX17" (with compiler
#   extensions), but CMake does not know the compile flags to use to enable it."
# then most likely a later version of GNU is required.
# In this event that GNU is located elsewhere, prior to running cmake,
# run the following (your location may vary):
# $ export CXX=/usr/local/bin/g++
# $ export CC=/usr/local/bin/gcc (optional)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(NFIR_ITL)

# if(USE_OPENCV)
#   project(NFIR_ITL VERSION 0.2.1)
# else()
#   project(NFIR_ITL VERSION 0.2.1)
# endif()

message(STATUS "ROOT PROJECT_NAME: ${PROJECT_NAME}")

# Pick up the library and binary
add_subdirectory(src/lib)
add_subdirectory(src/bin)
