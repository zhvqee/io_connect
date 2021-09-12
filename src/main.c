#include <stdio.h>

#include "nio/acceptors/acceptor.h"
#include "nio/entry/NioServer.h"


#define  SERVER_PORT  8888

int main() {

    struct NioServer nioServer;
    printf("123\n");
    initNioServer(&nioServer, SERVER_PORT);
}
