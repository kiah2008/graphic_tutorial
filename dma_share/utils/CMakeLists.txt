project(utils VERSION 0.1.0)

add_library(${PROJECT_NAME} STATIC
ipc/socket.c)

target_include_directories(${PROJECT_NAME}
PRIVATE
${CMAKE_CURRENT_SOURCE_DIR}/include
)