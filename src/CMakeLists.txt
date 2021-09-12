cmake_minimum_required(VERSION 3.17)
project(io_connect C)

set(CMAKE_C_STANDARD 99)


add_executable(io_connect main.c logs/log.c logs/log.h  nio/callback.c nio/callback.h nio/event.h nio/event.c kqueuedispatch.c nio/global.h nio/acceptor.c nio/acceptor.h nio/global.c nio/global.h nio/entry/NioServer.c nio/entry/NioServer.h thread/thread.c thread/thread.h nio/channel.c nio/channel.h nio/subreactor.c nio/subreactor.h)
