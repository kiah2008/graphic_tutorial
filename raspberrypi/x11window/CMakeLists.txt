project(x11window VERSION 0.1.0)

find_package(PkgConfig REQUIRED)
message(STATUS "=== PKG_CONFIG_FOUND: ${PKG_CONFIG_FOUND}")

pkg_check_modules(EGL REQUIRED egl)
pkg_check_modules(GLES REQUIRED glesv2)
pkg_check_modules(X11 REQUIRED x11)

add_executable(${PROJECT_NAME} x11_gl.c)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    "${EGL_INCLUDE_DIR}"
    "${GLES_INCLUDE_DIR}"
    "${X11_INCLUDE_DIR}"
    "${UTILS_INC}"
)

message(STATUS "==== gles ${GLES_INCLUDE_DIR}.")
message(STATUS "==== x11 ${X11_INCLUDE_DIR}.")
target_link_libraries(${PROJECT_NAME}
    "${GLES_LIBRARIES}"
    "${EGL_LIBRARIES}"
    "${X11_LIBRARIES}"
)

