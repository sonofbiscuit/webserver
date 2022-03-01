cmake_minimum_required(VERSION 3.21)
project(webserver)

set(CMAKE_CXX_STANDARD 17)

include_directories(webserver)

add_executable(webserver
        webserver/log.cc
        webserver/log.h)
