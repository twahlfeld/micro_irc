cmake_minimum_required(VERSION 3.3)
project(HW1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    server.c
    server.h auth.c auth.h list.c list.h srv_io.c srv_io.h client.c )

add_executable(HW1 ${SOURCE_FILES})