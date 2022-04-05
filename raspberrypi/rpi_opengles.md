# introduction
foucus on opengles(mesa/gbm/drm), vulkan, opencv programming and samples.

# pre-requirement
base on **raspberrypi 4b with bullseye**
```
$ lsb_release -a
No LSB modules are available.
Distributor ID: Debian
Description:    Debian GNU/Linux 11 (bullseye)
Release:        11
Codename:       bullseye
```

Arm arch **aarch64**
```
uname -a
Linux wethink 5.15.30-v8+ #1536 SMP PREEMPT Mon Mar 28 13:53:14 BST 2022 aarch64 GNU/Linux
```

# opengles

You need a GCC compiler, GDM, EGL, and GLES libraries. The GCC compiler is already included in the Raspbian image. 

## official libs
You can check if you have the GCC installed by executing gcc --version. You can also check if the GLES and EGL are installed by executing ls /opt/vc/lib which should list libbrcmEGL.so and libbrcmGLESv2.so. They are all already included in the Jessie/Stretch/Buster Raspbian! If you don't have libbrcmEGL.so, you will have to use the libEGL.so instead, which is located in the same folder.

## insall deps libs
```
sudo apt-get install libegl1-mesa-dev libgbm-dev libgles2-mesa-dev
```
如果需要使用drm, 则需要一并安装libdrm-dev

# 判断gl相关安装信息
## dpkg
```
pi@wethink:~ $ apt list --installed |grep -i gles
libgles-dev/stable,now 1.3.2-1 arm64 [installed,automatic]
libgles1/stable,now 1.3.2-1 arm64 [installed]
libgles2-mesa-dev/stable,now 20.3.5-1+rpt3+rpi1 arm64 [installed]
libgles2-mesa/stable,now 20.3.5-1+rpt3+rpi1 arm64 [installed]
libgles2/stable,now 1.3.2-1 arm64 [installed,automatic]
pi@wethink:~ $ dpkg -L libgles2
/.
/usr
/usr/lib
/usr/lib/aarch64-linux-gnu
/usr/lib/aarch64-linux-gnu/libGLESv2.so.2.1.0
/usr/share
/usr/share/bug
/usr/share/bug/libgles2
/usr/share/bug/libgles2/control
/usr/share/doc
/usr/share/doc/libgles2
/usr/share/doc/libgles2/changelog.Debian.gz
/usr/share/doc/libgles2/copyright
/usr/share/lintian
/usr/share/lintian/overrides
/usr/share/lintian/overrides/libgles2
/usr/lib/aarch64-linux-gnu/libGLESv2.so.2
```

详细可以参考
```
pi@wethink:/usr/lib/aarch64-linux-gnu/pkgconfig $ ls
bcm_host.pc   geany.pc      gtk-engines-2.pc        libcrypt.pc          libdrm.pc         libnsl.pc     lxappearance.pc      python-3.9.pc     xau.pc
egl.pc        glesv1_cm.pc  gtk-engines-pixflat.pc  libdrm_amdgpu.pc     libdrm_radeon.pc  libpng16.pc   mmal.pc              python3-embed.pc  xcb.pc
expat.pc      glesv2.pc     libbrotlicommon.pc      libdrm_etnaviv.pc    libdrm_tegra.pc   libpng.pc     opengl.pc            python3.pc        xdmcp.pc
freetype2.pc  gl.pc         libbrotlidec.pc         libdrm_freedreno.pc  libdrm_vc4.pc     libtirpc.pc   pthread-stubs.pc     vcsm.pc           zlib.pc
gbm.pc        glx.pc        libbrotlienc.pc         libdrm_nouveau.pc    libglvnd.pc       libxcrypt.pc  python-3.9-embed.pc  x11.pc
```

```
 dpkg-query -l libdrm-dev
Desired=Unknown/Install/Remove/Purge/Hold
| Status=Not/Inst/Conf-files/Unpacked/halF-conf/Half-inst/trig-aWait/Trig-pend
|/ Err?=(none)/Reinst-required (Status,Err: uppercase=bad)
||/ Name             Version      Architecture Description
+++-================-============-============-================================>
ii  libdrm-dev:arm64 2.4.104-1    arm64        Userspace interface to kernel DR>
```

# Opencv

## install opencv on rpi4
```
#!/bin/bash
set -e
echo "Installing OpenCV 4.5.5 on your Raspberry Pi 64-bit OS"
echo "It will take minimal 2.0 hour !"
cd ~
# install the dependencies
sudo apt-get install -y build-essential cmake git unzip pkg-config
sudo apt-get install -y libjpeg-dev libtiff-dev libpng-dev
sudo apt-get install -y libavcodec-dev libavformat-dev libswscale-dev
sudo apt-get install -y libgtk2.0-dev libcanberra-gtk* libgtk-3-dev
sudo apt-get install -y libgstreamer1.0-dev gstreamer1.0-gtk3
sudo apt-get install -y libgstreamer-plugins-base1.0-dev gstreamer1.0-gl
sudo apt-get install -y libxvidcore-dev libx264-dev
sudo apt-get install -y python3-dev python3-numpy python3-pip
sudo apt-get install -y libtbb2 libtbb-dev libdc1394-22-dev
sudo apt-get install -y libv4l-dev v4l-utils
sudo apt-get install -y libopenblas-dev libatlas-base-dev libblas-dev
sudo apt-get install -y liblapack-dev gfortran libhdf5-dev
sudo apt-get install -y libprotobuf-dev libgoogle-glog-dev libgflags-dev
sudo apt-get install -y protobuf-compiler

# download the latest version
cd ~ 
sudo rm -rf opencv*
wget -O opencv.zip https://github.com/opencv/opencv/archive/4.5.5.zip 
wget -O opencv_contrib.zip https://github.com/opencv/opencv_contrib/archive/4.5.5.zip 
# unpack
unzip opencv.zip 
unzip opencv_contrib.zip 
# some administration to make live easier later on
mv opencv-4.5.5 opencv
mv opencv_contrib-4.5.5 opencv_contrib
# clean up the zip files
rm opencv.zip
rm opencv_contrib.zip

# set install dir
cd ~/opencv
mkdir build
cd build

# run cmake
cmake -D CMAKE_BUILD_TYPE=RELEASE \
-D CMAKE_INSTALL_PREFIX=/usr/local \
-D OPENCV_EXTRA_MODULES_PATH=~/opencv_contrib/modules \
-D ENABLE_NEON=ON \
-D WITH_OPENMP=ON \
-D WITH_OPENCL=OFF \
-D BUILD_TIFF=ON \
-D WITH_FFMPEG=ON \
-D WITH_TBB=ON \
-D BUILD_TBB=ON \
-D WITH_GSTREAMER=ON \
-D BUILD_TESTS=OFF \
-D WITH_EIGEN=OFF \
-D WITH_V4L=ON \
-D WITH_LIBV4L=ON \
-D WITH_VTK=OFF \
-D WITH_QT=OFF \
-D OPENCV_ENABLE_NONFREE=ON \
-D INSTALL_C_EXAMPLES=OFF \
-D INSTALL_PYTHON_EXAMPLES=OFF \
-D PYTHON3_PACKAGES_PATH=/usr/lib/python3/dist-packages \
-D OPENCV_GENERATE_PKGCONFIG=ON \
-D BUILD_EXAMPLES=OFF ..

# run make
make -j4
sudo make install
sudo ldconfig

# cleaning (frees 300 MB)
make clean
sudo apt-get update

echo "Congratulations!"
echo "You've successfully installed OpenCV 4.5.5 on your Raspberry Pi 64-bit OS"
```


# cross compile with sysroot
introduce how to cross compiling rpi4 binary in unix-like host.

## install your gcc and g++ for aarch64

```
sudo apt-get install build-essential gdb
sudo apt-get install gcc-aarch64-linux-gnu
sudo apt install g++-9-aarch64-linux-gnu
```

## using sshfs to remount rpi's root to local path
refer to [using sshfs](https://github.com/kiah2008/kiah2008/blob/main/2022/tools/mount%20remote%20linux%20directory%20to%20local%20fs%20using%20sshfs.md).
recommend to using ssh's key to avoid input user info.

```
pi@192.168.2.107:/ on /home/kiah/rpi_root type fuse.sshfs (rw,nosuid,nodev,relatime,user_id=1000,group_id=1000)
```
## add toochain cmake file for your cmake project

```
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_CROSSCOMPILING TRUE)


#using sshfs to mount remote rpi:/usr/ to local rpi_usr

#set(RPI_ROOT "/home/kiah/rpi_root")

set(CMAKE_SYSROOT ${RPI_ROOT})
#set(CMAKE_PREFIX_PATH ${RPI_ROOT})

include_directories(${RPI_ROOT}/usr/include/libdrm)
# Name of C compiler.
set(CMAKE_C_COMPILER "/usr/bin/aarch64-linux-gnu-gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/aarch64-linux-gnu-g++-9")

# Where to look for the target environment. (More paths can be added here)
set(CMAKE_FIND_ROOT_PATH "${RPI_ROOT}/usr/aarch64-linux-gnu")
#set(CMAKE_SYSROOT /usr/aarch64-linux-gnu)
set(CMAKE_INCLUDE_PATH  ${RPI_ROOT}/usr/include/aarch64-linux-gnu)
set(CMAKE_LIBRARY_PATH  ${RPI_ROOT}/usr/lib/aarch64-linux-gnu)
set(CMAKE_PROGRAM_PATH  ${RPI_ROOT}/usr/bin/aarch64-linux-gnu)

# Adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment only.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# automatically use the cross-wrapper for pkg-config
SET(ENV{PKG_CONFIG_PATH} "${RPI_ROOT}/usr/share/pkgconfig/:${RPI_ROOT}/usr/lib/pkgconfig/:${RPI_ROOT}/usr/lib/aarch64-linux-gnu/pkgconfig/")
#SET(ENV{PKG_CONFIG_LIBDIR } ${RPI_ROOT}/lib/aarch64-linux-gnu/pkgconfig/)

message(STATUS "=== PKG_CONFIG_PATH: " $ENV{PKG_CONFIG_PATH})
set(PKG_CONFIG_EXECUTABLE "/usr/bin/pkg-config" CACHE FILEPATH "pkg-config executable")
```

## add toolchain file in vs code's settings.json
```
{
    "cmake.additionalKits": [
        "gnu-gcc"
    ],
    "cmake.configureArgs": [
        "-DCMAKE_TOOLCHAIN_FILE=${workspaceFolder}/makefiles/cmake/aarch64_build.cmake",
        "-DRPI_SYSROOT=/home/kiah/rpi_root"
    ],
    "C_Cpp.default.cppStandard": "c++14",
    "C_Cpp.default.cStandard": "c11",
    "cmake.configureOnOpen": true,
    "C_Cpp.errorSquiggles": "Disabled"
}
```

# using cmake to build
