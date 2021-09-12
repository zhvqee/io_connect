//
// Created by 朱琪 on 2021/9/10.
//

#ifndef IO_CONNECT_ACCEPTOR_H
#define IO_CONNECT_ACCEPTOR_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <fcntl.h>
#include "../../logs/log.h"
#include <strings.h>
#include "../global/global.h"
#include "../events/event.h"
#include "../../thread/thread.h"
#include "assert.h"

#define    LISTENQ        1024

typedef struct Acceptor {
    /**
     * 监听fd
     */
    int listenfd;

    /**
     * 监听端口
     */
    int listenPort;

    /**
     * 有自己的eventLoop.
     */
    NioEventLoop *nioEventLoop;
} Acceptor;


Acceptor *initAcceptor(int port,void *args);

void acceptEvent(Acceptor *);

#endif //IO_CONNECT_ACCEPTOR_H
