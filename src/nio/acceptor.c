//
// Created by 朱琪 on 2021/9/10.
//

#include "acceptor.h"
#include "event.h"
#include "entry/NioServer.h"
#include "channel.h"

#define ACCEPTOR_THREAD_NAME "main-acceptor-thread"

static int acceptReadHandle(void *args);

Acceptor *initAcceptor(int port, void *args) {
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


    // 初始化 event_loop thread
    acceptor->nioEventLoop = initNioEventLoop(ACCEPTOR_THREAD_NAME);


    //初始化完后，需要构建listenfd 的channel 通道，并注册到nioeventLoop 中
    Channel *channel = initChannel(acceptor->listenfd, NIO_EVENT_READ, acceptReadHandle, NULL, args);
    addNioEventLoopChannelEvent(acceptor->nioEventLoop, channel);

    return acceptor;
}


void acceptEvent(struct Acceptor *acceptor) {
    nioEventLoopRun(acceptor->nioEventLoop, 0);
}

/**
 * 实际接收连接
 * @param args
 * @return
 */
static int acceptReadHandle(void *args) {
    struct NioServer *nioServer = (struct NioServer *) args;
    Acceptor *acceptor = nioServer->acceptor;
    int listenfd = acceptor->listenfd;

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int clientFd = accept(listenfd, (struct sockaddr *) &client_addr, &client_len);
    makeSocketNoBlock(clientFd);

    NioEventLoop *workNioEventLoop = selectSubReactorWorkEventLoop(nioServer->subReactor);
    //目前只注册可读，当需要写时，在注册，否则 tcp 缓存区发送缓冲区有空，就会触发可写时间，epoll返回
    Channel *clientChannel = initChannel(clientFd, NIO_EVENT_READ, readChannel, writeChannel, (void *) clientFd);
    // 添加客户端连接到 workNioEventLoop
    addNioEventLoopChannelEvent(workNioEventLoop, clientChannel);
    return 0;
}
