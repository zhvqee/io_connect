cmake_minimum_required(VERSION 3.17)
project(io_connect C)

set(CMAKE_C_STANDARD 99)


add_executable(io_connect main.c logs/log.c logs/log.h  nio/callback.c nio/callback.h nio/events/event.h nio/events/event.c nio/subreactor/kqueuedispatch.c nio/global/global.h nio/acceptors/acceptor.c nio/acceptors/acceptor.h nio/global/global.c nio/global/global.h nio/entry/NioServer.c nio/entry/NioServer.h thread/thread.c thread/thread.h nio/channels/channel.c nio/channels/channel.h nio/subreactor/subreactor.c nio/subreactor/subreactor.h)
