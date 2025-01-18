cmake_minimum_required(VERSION 3.20)

project(vision_viewer)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/app)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_CXX_STANDARD 11)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Find OpenCV
find_package(OpenCV REQUIRED)
if(${OpenCV_FOUND})
    message(STATUS "OpenCV library status:")
    message(STATUS "  version: ${OpenCV_VERSION}")
    message(STATUS "  libraries: ${OpenCV_LIBS}")
    message(STATUS "  include path: ${OpenCV_INCLUDE_DIRS}")
else(${OpenCV_FOUND})
    message(WARNING "Cannot found OpenCV.")
endif(${OpenCV_FOUND})

# Add video_viewer
add_subdirectory(video_viewer)

# Add camera_viewer
add_subdirectory(camera_viewer)