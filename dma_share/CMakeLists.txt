project(x11window VERSION 0.1.0)

include(FindPkgConfig REQUIRED)

# find_package(PkgConfig REQUIRED)

# pkg_check_modules(REQUIRED wayland-egl)
pkg_check_modules(EGL REQUIRED egl)
pkg_check_modules(GLES REQUIRED glesv2)
pkg_check_modules(DRM REQUIRED libdrm)
pkg_check_modules(X11 REQUIRED x11)


add_executable(${PROJECT_NAME} main.c)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    "${MESA_EGL_INCLUDE_DIR}"
    "${EGL_INCLUDE_DIR}"
    "${GLES_INCLUDE_DIR}"
    "${DRM_INCLUDE_DIR}"
    ${CMAKE_SOURCE_DIR}/utils/include
)

target_link_libraries(${PROJECT_NAME}
    "${MESA_EGL_LIBRARIES}"
    "${GLES_LIBRARIES}"
    "${GL_LIBRARIES}"
    "${EGL_LIBRARIES}"
    "${X11_LIBRARIES}"
    -lGL
)

