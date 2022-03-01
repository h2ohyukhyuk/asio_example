## Introduction
This Boost ASIO example shows how to communicate images and some information between host and clients.

## Perequisites
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
  $ cmake -D CMAKE_BUILD_TYPE=RELEASE \
    -D CMAKE_INSTALL_PREFIX=/usr/local \
    -D WITH_TBB=OFF \
    -D WITH_IPP=OFF \
    -D WITH_1394=OFF \
    -D BUILD_WITH_DEBUG_INFO=OFF \
    -D BUILD_DOCS=OFF \
    -D INSTALL_C_EXAMPLES=ON \
    -D INSTALL_PYTHON_EXAMPLES=ON \
    -D BUILD_EXAMPLES=OFF \
    -D BUILD_TESTS=OFF \
    -D BUILD_PERF_TESTS=OFF \
    -D WITH_QT=OFF \
    -D WITH_GTK=ON \
    -D WITH_OPENGL=ON \
    -D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib-4.2.0/modules \
    -D WITH_V4L=ON  \
    -D WITH_FFMPEG=ON \
    -D WITH_XINE=ON \
    -D BUILD_NEW_PYTHON_SUPPORT=ON \
    -D OPENCV_GENERATE_PKGCONFIG=ON ../
  

  3.2.0
  ```