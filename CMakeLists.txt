cmake_minimum_required(VERSION 3.0.0)
project(graphic_tutorial VERSION 0.1.0)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -undefined")

add_subdirectory(${CMAKE_SOURCE_DIR}/misc/)
add_subdirectory(${CMAKE_SOURCE_DIR}/3rdLibs/libyuv/)
message(STATUS "${CMAKE_SYSTEM_PROCESSOR}.")
if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64")
add_subdirectory(${CMAKE_SOURCE_DIR}/raspberrypi/)
endif()


