cmake_minimum_required(VERSION 3.17)
project(io_connect C)

set(CMAKE_C_STANDARD 99)

add_executable(io_connect main.c csocket.h csocket.c logs/log.c logs/log.h event.h event.c kqueuedispatch.c global.h)