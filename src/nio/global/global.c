//
// Created by 朱琪 on 2021/9/10.
//

#include "global.h"
#include "fcntl.h"

void makeSocketNoBlock(int socketfd){
    int flags = fcntl(socketfd, F_GETFL, 0);
    fcntl(socketfd, F_SETFL, flags | O_NONBLOCK);
}
