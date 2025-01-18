cmake_minimum_required(VERSION 3.20)

project(endo_viewer_cv)

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

# Build target
file(GLOB_RECURSE SRC_CPP ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_executable(${PROJECT_NAME}
    main.cpp
    ${SRC_CPP}
)
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${OpenCV_INCLUDE_DIRS}>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/>
        
)
target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
)
