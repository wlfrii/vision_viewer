project(camera_viewer)

# Build target
file(GLOB_RECURSE SRC_CPP ${CMAKE_CURRENT_SOURCE_DIR}/../src/*.cpp)
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
