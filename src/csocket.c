//
// Created by zhuqi on 2021/9/8.
//

#include "csocket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <fcntl.h>
#include "logs/log.h"

struct sockaddr_in *buildSockAddr(int port) {
    struct sockaddr_in si;
    si.sin_addr.s_addr = htonl(INADDR_ANY);
    si.sin_family = AF_INET;
    si.sin_port = htons(port);
    return &si;
}

int createNoBlockSocket(int port, int backlog) {
    int serverSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (serverSocket <= 0) {
        log(ERROR, "%s", "create socket failed !");
        return -1;
    }
    struct sockaddr_in *sip = buildSockAddr(port);
    int ret = bind(serverSocket, (struct sockaddr *) sip, sizeof(sip));
    if (ret < 0) {
        log(ERROR, "socket fd %d bind failed!", serverSocket);
        return -1;
    }

    int flags = fcntl(socket, F_GETFL, 0);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);

    if (listen(serverSocket, backlog) != 0) {
        log(ERROR, "socket fd %d listen failed!", serverSocket);
        return -1;
    }
    return serverSocket;
}

int accept(int serverSocket,struct  sockaddr_storage* storage,socklen_t* len){

}


