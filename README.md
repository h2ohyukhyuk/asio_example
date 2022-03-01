## Introduction
This Boost ASIO example shows how to communicate images and some information between host and clients.

## Perequisites
  - boost
  ```bash
  $ sudo apt-get install libboost-all-dev
  $ dpkg -s libboost-dev | grep 'Version'
  Version: 1.65.1.0ubuntu1
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
