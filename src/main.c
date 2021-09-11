#include <stdio.h>

#include "nio/acceptor.h"
#include "nio/entry/NioServer.h"


#define  SERVER_PORT  8888

int main() {

    struct NioServer nioServer;

    initNioServer(&nioServer, SERVER_PORT);
}
