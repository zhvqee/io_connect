//
// Created by 朱琪 on 2021/9/10.
//

#include "NioServer.h"


void initNioServer(struct NioServer *nioServer, int listenPort) {

    struct SubReactor *subReactor = initSubReactor();
    struct Acceptor *acceptor = initAcceptor(listenPort, nioServer);

    nioServer->acceptor = acceptor;
    nioServer->subReactor = subReactor;

    // 开始接受 socket 连接
    acceptEvent(acceptor);

}
