//
// Created by 朱琪 on 2021/9/10.
//

#ifndef IO_CONNECT_NIOSERVER_H
#define IO_CONNECT_NIOSERVER_H

#include "../acceptor.h"
#include "../subreactor.h"

/**
 * 入口，存储了全局变量
 */
typedef struct NioServer {
    struct Acceptor *acceptor;

    struct SubReactor *subReactor;

} NioServer;

/**
 * 初始化nio Server
 * @param nioServer
 * @param acceptor
 */
void initNioServer(struct NioServer *nioServer,
                   int listenPort);

#endif //IO_CONNECT_NIOSERVER_H
