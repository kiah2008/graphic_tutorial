cmake_minimum_required(VERSION 3.0.0)

# include(${CMAKE_SOURCE_DIR}/makefiles/cmake/aarch64_build.cmake)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/x11window)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/headless-gl)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/rpi4_compute)
