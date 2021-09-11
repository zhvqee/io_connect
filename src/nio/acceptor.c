//
// Created by 朱琪 on 2021/9/10.
//

#include "acceptor.h"
#include "event.h"

#define ACCEPTOR_THREAD_NAME "main-acceptor-thread"

Acceptor *initAcceptor(int port) {
    Acceptor *acceptor = (Acceptor *) malloc(sizeof(Acceptor));
    acceptor->listenPort = port;

    // 创建socket
    acceptor->listenfd = socket(PF_INET, SOCK_STREAM, 0);
    makeSocketNoBlock(acceptor->listenPort);


    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    int on = 1;
    setsockopt(acceptor->listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    int rt1 = bind(acceptor->listenfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (rt1 < 0) {
        io_log(ERROR, "bind failed ");
    }

    int rt2 = listen(acceptor->listenfd, LISTENQ);
    if (rt2 < 0) {
        io_log(ERROR, "listen failed ");
    }


    // 初始化 event_loop;
    acceptor->nioEventLoop = initNioEventLoop(ACCEPTOR_THREAD_NAME);


    //初始化完后，需要构建listenfd 的channel 通道，并注册到nioeventLoop 中
    Channel *channel = initChannel(acceptor->listenfd, NIO_EVENT_READ, NULL, NULL, NULL);
    addNioEventLoopChannelEvent(acceptor->nioEventLoop, channel);

    return acceptor;
}

void acceptEvent(struct Acceptor *acceptor) {
    nioEventLoopRun(acceptor->nioEventLoop);
}
