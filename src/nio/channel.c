//
// Created by 朱琪 on 2021/9/11.
//

#include "channel.h"
#include "stdio.h"
#include <unistd.h>


int readChannel(void *args) {
    int fd = (int) args;
    char recv[1024]={};
    ssize_t cnt = read(fd, recv, 1024);
    printf("%d,%s\n", cnt, recv);
}

int writeChannel(void *args) {

}