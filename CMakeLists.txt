cmake_minimum_required(VERSION 3.0)

# Include the common header files
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/jsoncpp/)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)

