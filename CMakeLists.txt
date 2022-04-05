cmake_minimum_required(VERSION 3.0.0)
project(X11Window VERSION 0.1.0)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/)

#add_subdirectory(${CMAKE_SOURCE_DIR}/dma_share)
# add_subdirectory(${CMAKE_SOURCE_DIR}/raspberrypi/opengles)
add_subdirectory(${CMAKE_SOURCE_DIR}/raspberrypi/)


