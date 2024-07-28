cmake_minimum_required(VERSION 3.10)
project(reator_server VERSION 1.0)

# Set the default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Set compiler flags for Release mode
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  add_compile_options(-O3 -DNDEBUG)
endif()
# Set compiler flags for Debug mode
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-g -D_DEBUG)
endif()


# Set output directories for executables and libraries
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib")


add_executable(server src/server.cpp)
target_link_libraries(server pthread)

add_executable(client src/client.cpp)
target_link_libraries(client pthread)
