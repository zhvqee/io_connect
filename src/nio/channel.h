//
// Created by 朱琪 on 2021/9/11.
//

#ifndef IO_CONNECT_CHANNEL_H
#define IO_CONNECT_CHANNEL_H

#include "event.h"
#include "sys/socket.h"

int readChannel(void *args);

int writeChannel(void *args);

#endif //IO_CONNECT_CHANNEL_H
