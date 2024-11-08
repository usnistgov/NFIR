
project(${PROJECT_NAME})

option(USE_NFIMM "Enable NFIMM" ON)

FILE(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_library( ${PROJECT_NAME} ${sources} )
#   filter_mask.cpp
#   filter_mask_gaussian.cpp
#   filter_mask_ideal.cpp
#   nfir_lib.cpp
#   resample.cpp
#   resample_down.cpp
#   resample_up.cpp
# )


if(_WIN32_64)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
  message(STATUS "LIB: CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

  # To access OpenCV, use either these two lines below:
  include(D:/OpenCV/opencv-4.5.0/build/x64/vc15/lib/OpenCVConfig.cmake)
  target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

  # Or to access OpenCV, use these two lines below:
  # set(OpenCV_DIR "D:/OpenCV/opencv-4.5.0/build/x64/vc15/lib/")
  # find_package(OpenCV REQUIRED)

  message(STATUS "OpenCV_INCLUDE_DIRS found as: '${OpenCV_INCLUDE_DIRS}'")
  message(STATUS "OpenCV_LIBS found as: '${OpenCV_LIBS}'")

  # line below ALWAYS required
  target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS} )

  if(USE_NFIMM)
    target_compile_definitions(${PROJECT_NAME} PUBLIC USE_NFIMM)
    target_include_directories(${PROJECT_NAME} PRIVATE
    /YOUR-DIR-HERE/NFIMM/src/include )
    target_link_libraries(${PROJECT_NAME} R:/NFIMM_ITL/src/lib/Release/NFIMM_ITL.lib)
  endif()
  
else()
  message(STATUS "LIB: CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(OPENCV REQUIRED opencv4)
	target_link_libraries(${PROJECT_NAME} ${OPENCV_LINK_LIBRARIES})
	target_include_directories(${PROJECT_NAME} PUBLIC ${OPENCV_INCLUDE_DIRS})
	target_compile_options(${PROJECT_NAME} PUBLIC ${OPENCV_CFLAGS_OTHER})

  if(USE_NFIMM)
    target_compile_definitions(${PROJECT_NAME} PUBLIC USE_NFIMM)
    target_include_directories(${PROJECT_NAME} PRIVATE
    /YOUR-DIR-HERE/NFIMM/src/include)
    target_link_libraries(${PROJECT_NAME} /YOUR-DIR-HERE/libNFIMM_ITL.a)
  endif()

  message(STATUS "OPENCV_INCLUDE_DIRS found as: '${OPENCV_INCLUDE_DIRS}'")
	message(STATUS "OPENCV_LIBRARIES found as: '${OPENCV_LINK_LIBRARIES}'")
endif()

message(STATUS "LIB: CMAKE_CURRENT_SOURCE_DIR: '${CMAKE_CURRENT_SOURCE_DIR}'")
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
get_property(inc_dirs TARGET ${PROJECT_NAME} PROPERTY INCLUDE_DIRECTORIES)
message(STATUS "${PROJECT_NAME} include dirs =>> ${inc_dirs}")
