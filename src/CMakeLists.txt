cmake_minimum_required(VERSION 3.17)
project(io_connect C)

set(CMAKE_C_STANDARD 99)

add_executable(io_connect main.c csocket.h csocket.c logs/log.c logs/log.h nio/callback.c nio/callback.h nio/default.h)