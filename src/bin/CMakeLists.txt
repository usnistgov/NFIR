project(NFIR_bin)

if(_WIN32_64)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_64")

  # message(STATUS "BIN: CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
  # message(STATUS "BIN: CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
  add_executable( ${PROJECT_NAME}
    nfir.cpp
    ../../thirdparty/szx/glob/glob_win32.cpp
  )
else()
  # message(STATUS "BIN: CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
  # message(STATUS "BIN: CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
  add_executable( ${PROJECT_NAME}
    nfir.cpp
    ../../thirdparty/szx/glob/glob_posix.cpp
  )
endif()

message(STATUS "BIN: CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "BIN: CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(NFIR_bin NFIR_ITL)
include_directories(${PROJECT_NAME}  ${CMAKE_CURRENT_SOURCE_DIR}/../include ${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/szx/glob)

get_property(inc_dirs TARGET ${PROJECT_NAME} PROPERTY INCLUDE_DIRECTORIES)
message(STATUS "${PROJECT_NAME} include dirs =>> ${inc_dirs}")
