set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_CROSSCOMPILING TRUE)


#using sshfs to mount remote rpi:/usr/ to local rpi_usr
# if (NOT ENV{RPI_SYSROOT})
# message(FATAL_ERROR "please point rpi sysroot")
# endif()

# set(RPI_SYSROOT "/home/kiah/rpi_root")

set(CMAKE_SYSROOT ${RPI_SYSROOT})
#set(CMAKE_PREFIX_PATH ${RPI_SYSROOT})

include_directories(${RPI_SYSROOT}/usr/include/libdrm)
# Name of C compiler.
set(CMAKE_C_COMPILER "/usr/bin/aarch64-linux-gnu-gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/aarch64-linux-gnu-g++-9")

# Where to look for the target environment. (More paths can be added here)
set(CMAKE_FIND_ROOT_PATH "${RPI_SYSROOT}/usr/aarch64-linux-gnu")
#set(CMAKE_SYSROOT /usr/aarch64-linux-gnu)
set(CMAKE_INCLUDE_PATH  ${RPI_SYSROOT}/usr/include/aarch64-linux-gnu)
set(CMAKE_LIBRARY_PATH  ${RPI_SYSROOT}/usr/lib/aarch64-linux-gnu)
set(CMAKE_PROGRAM_PATH  ${RPI_SYSROOT}/usr/bin/aarch64-linux-gnu)

# Adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment only.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# automatically use the cross-wrapper for pkg-config
SET(ENV{PKG_CONFIG_PATH} "${RPI_SYSROOT}/usr/share/pkgconfig/:${RPI_SYSROOT}/usr/lib/pkgconfig/:${RPI_SYSROOT}/usr/lib/aarch64-linux-gnu/pkgconfig/")
#SET(ENV{PKG_CONFIG_LIBDIR } ${RPI_SYSROOT}/lib/aarch64-linux-gnu/pkgconfig/)

message(STATUS "=== PKG_CONFIG_PATH: " $ENV{PKG_CONFIG_PATH})
set(PKG_CONFIG_EXECUTABLE "/usr/bin/pkg-config" CACHE FILEPATH "pkg-config executable")