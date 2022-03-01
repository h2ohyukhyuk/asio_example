cmake_minimum_required(VERSION 3.18)

project(ASIO LANGUAGES CXX)

add_compile_options(-O3)

aux_source_directory(src_server SRC_SERVER)
aux_source_directory(src_client SRC_CLIENT)
aux_source_directory(common SRC_COMMON)

find_package(Boost REQUIRED system)
find_package(Threads)
find_package(OpenCV REQUIRED)

include_directories(${Boost_INCLUDE_DIR})
include_directories(${OpenCV_INCLUDE_DIR})

add_executable(server ${SRC_SERVER} ${SRC_COMMON})
add_executable(client ${SRC_CLIENT} ${SRC_COMMON})

target_link_libraries(client
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${OpenCV_LIBS}
)
 
target_link_libraries(server
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${OpenCV_LIBS}
)