project(triangle VERSION 0.1.0)

find_package(PkgConfig REQUIRED)

pkg_check_modules(DRM REQUIRED libdrm)
pkg_check_modules(GBM REQUIRED gbm)
pkg_check_modules(EGL REQUIRED egl)
pkg_check_modules(GLES REQUIRED glesv2)
pkg_check_modules(X11 REQUIRED x11)

message(STATUS "=== ${DRM_INCLUDE_DIR}")

add_executable(${PROJECT_NAME} triangle.c)

message(STATUS "==== ${DRM_CFLAGS} ${GBM_CFLAGS} ${EGL_CFLAGS} ${GLES_CFLAGS}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g ${DRM_CFLAGS} ${GBM_CFLAGS} ${EGL_CFLAGS} ${GLES_CFLAGS}")

target_include_directories(${PROJECT_NAME}
    PRIVATE
    "${DRM_INCLUDE_DIR}"
    "${EGL_INCLUDE_DIR}"
    "${GLES_INCLUDE_DIR}"
    "${X11_INCLUDE_DIR}"
    "${GBM_INCLUDE_DIR}"
)

target_link_libraries(${PROJECT_NAME}
    "${DRM_LIBRARIES}"
    "${GBM_LIBRARIES}"
    "${EGL_LIBRARIES}"
    "${GLES_LIBRARIES}"
    "${X11_LIBRARIES}"
)

