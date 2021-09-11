//
// Created by 朱琪 on 2021/9/10.
//

#include "NioServer.h"


void initNioServer(struct NioServer *nioServer, int listenPort) {

    struct Acceptor *acceptor = initAcceptor(listenPort);
    nioServer->acceptor = acceptor;
    int enableCPUNum = sysconf(_SC_NPROCESSORS_ONLN);
    nioServer->threadPool = beginThreadPool(enableCPUNum * 2, "nio-server-thread");
    // 开始接受 socket 连接
    acceptEvent(acceptor);

}
