cmake_minimum_required(VERSION 3.17)
project(io_connect C)

set(CMAKE_C_STANDARD 99)

<<<<<<< HEAD
add_executable(io_connect main.c csocket.h csocket.c logs/log.c logs/log.h nio/callback.c nio/callback.h nio/default.h)
=======
add_executable(io_connect main.c csocket.h csocket.c logs/log.c logs/log.h event.h event.c kqueuedispatch.c global.h)
>>>>>>> 4afb50bdc812ffcf19ee1db52d09efc8f212183e
