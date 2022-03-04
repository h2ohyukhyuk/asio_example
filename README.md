## Introduction
This Boost ASIO example shows how to communicate images and some information between host and clients.

## Perequisites
  - OS
  ```bash
  $ lsb_release -a
  No LSB modules are available.
  Distributor ID:	Ubuntu
  Description:	Ubuntu 18.04.6 LTS
  Release:	18.04
  Codename:	bionic
  ```
  - cmake
  ```bash
  $ sudo apt-get install libssl-dev openssl
  $ wget https://github.com/Kitware/CMake/release/download/v3.19.2/cmake-3.19.2.tar.gz
  $ tar -xvf cmake-3.19.2.tar.gz
  $ cd cmake-3.19.2
  $ ./bootstrap
  $ make -j8
  $ sudo make install
  $ cmake --version
  cmake version 3.19.2
  ```
  - boost
  ```bash
  $ sudo apt-get install libboost-all-dev
  $ dpkg -s libboost-dev | grep 'Version'
  Version: 1.65.1.0ubuntu1
  ```
  - opencv
  ```bash
  $ sudo apt update
  $ sudo apt install python3-opencv
  $ pkg-config --modversion opencv
  3.2.0
  ```
## Socket buff
  - check current buffer size
  ```bash
  $ sysctl -a | grep "net.core.rmem\|net.core.wmem"
  $ cat /proc/sys/net/ipv4/tcp_rmem
  $ cat /proc/sys/net/ipv4/tcp_wmem
  ```

  - modify buffer size
  ```bash

  # 500MB
sudo sysctl -w net.ipv4.tcp_window_scaling="1"
sudo sysctl -w net.core.rmem_default="536862720"
sudo sysctl -w net.core.wmem_default="536862720"
sudo sysctl -w net.core.rmem_max="536862720"
sudo sysctl -w net.core.wmem_max="536862720"
  # min: 500MB, default: 500MB, max: 1GB
sudo sysctl -w net.ipv4.tcp_rmem="536862720 1073725440 1073725440"
sudo sysctl -w net.ipv4.tcp_wmem="536862720 1073725440 1073725440"
  # min: 8GB, default: 16GB, max: 16GB (unit: 4,096 bytes)
sudo sysctl -w net.ipv4.tcp_mem="2097152 4194304 4194304"

  $ sudo gedit /etc/sysctl.conf  
  # 500MB
  net.ipv4.tcp_window_scaling="1"
  net.core.rmem_default="536862720"
  net.core.wmem_default="536862720"
  net.core.rmem_max="536862720"
  net.core.wmem_max="536862720"
  # min: 500MB, default: 500MB, max: 1GB
  net.ipv4.tcp_rmem="536862720 1073725440 1073725440"
  net.ipv4.tcp_wmem="536862720 1073725440 1073725440"
  # min: 8GB, default: 16GB, max: 16GB (unit: 4,096 bytes)
  net.ipv4.tcp_mem="2097152 4194304 4194304"
  ```

## Build
```bash
  $ cd asio_example
  $ mkdir build
  $ cd build
  $ cmake ..
  $ make
```

## Run
  - server
  ```bash
  $ ./server 8080
  ```
  - client
  ```bash
  $ ./client 127.0.0.1 8080
  ```
## Build Opencv from src
  ```bash
  $ cat /proc/cpuinfo | grep processor | wc -l
  $ pkg-config --modversion opencv
  $ sudo apt-get purge  libopencv* python-opencv
  $ sudo apt-get autoremove
  $ sudo find /usr/local/ -name "*opencv*" -exec rm -i {} \;
  $ sudo apt-get update
  $ sudo apt-get upgrade
  $ sudo apt-get install build-essential cmake
  $ sudo apt-get install pkg-config
  $ sudo apt-get install libjpeg-dev libtiff5-dev libpng-dev
  $ sudo apt-get install libavcodec-dev libavformat-dev libswscale-dev libxvidcore-dev libx264-dev libxine2-dev
  $ sudo apt-get install libv4l-dev v4l-utils
  $ sudo apt-get install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev
  $ sudo apt-get install libgtk2.0-dev 
  another option libgtk-3-dev libqt4-dev libqt5-dev
  $ sudo apt-get install mesa-utils libgl1-mesa-dri libgtkgl2.0-dev libgtkglext1-dev
  $ sudo apt-get install libatlas-base-dev gfortran libeigen3-dev
  $ sudo apt-get install python2.7-dev python3-dev python-numpy python3-numpy

  $ mkdir opencv
  $ cd opencv
  $ mkdir opencv
  $ wget -O opencv.zip https://github.com/opencv/opencv/archive/4.2.0.zip
  $ unzip opencv.zip
  $ wget -O opencv_contrib.zip https://github.com/opencv/opencv_contrib/archive/4.2.0.zip
  $ unzip opencv_contrib.zip
  $ ls -d */
  opencv-4.2.0/  opencv_contrib-4.2.0/
  $ cd opencv-4.2.0
  $ mkdir build
  $ cd build
  $ cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D WITH_TBB=OFF -D WITH_IPP=OFF -D WITH_1394=OFF -D BUILD_WITH_DEBUG_INFO=OFF -D BUILD_DOCS=OFF -D INSTALL_C_EXAMPLES=ON -D INSTALL_PYTHON_EXAMPLES=ON -D BUILD_EXAMPLES=OFF -D BUILD_TESTS=OFF -D BUILD_PERF_TESTS=OFF -D WITH_QT=OFF -D WITH_GTK=ON -D WITH_OPENGL=ON -D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib-4.2.0/modules -D WITH_V4L=ON  -D WITH_FFMPEG=ON -D WITH_XINE=ON -D BUILD_NEW_PYTHON_SUPPORT=ON -D OPENCV_GENERATE_PKGCONFIG=ON ../

  when python path is wrong, add below
  -D PYTHON2_INCLUDE_DIR=/usr/include/python2.7 -D PYTHON2_NUMPY_INCLUDE_DIRS=/usr/lib/python2.7/dist-packages/numpy/core/include/ -D PYTHON2_PACKAGES_PATH=/usr/lib/python2.7/dist-packages -D PYTHON2_LIBRARY=/usr/lib/x86_64-linux-gnu/libpython2.7.so -D PYTHON3_INCLUDE_DIR=/usr/include/python3.6m -D PYTHON3_NUMPY_INCLUDE_DIRS=/usr/lib/python3/dist-packages/numpy/core/include/ -D PYTHON3_PACKAGES_PATH=/usr/lib/python3/dist-packages -D PYTHON3_LIBRARY=/usr/lib/x86_64-linux-gnu/libpython3.6m.so ../
  $ time make -j8
  $ sudo make install
  $ cat /etc/ld.so.conf.d/*
  $ sudo sh -c 'echo '/usr/local/lib' > /etc/ld.so.conf.d/opencv.conf'
  $ sudo ldconfig
  ```