//
// Created by 朱琪 on 2021/9/11.
//

#ifndef IO_CONNECT_SUBREACTOR_H
#define IO_CONNECT_SUBREACTOR_H

#include "../events/event.h"
#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"
#include "unistd.h"
#include "strings.h"

/**
 * N 个工作线程
 */
typedef struct SubReactor {

    pthread_mutex_t mutex;

    NioEventLoop **nioEventLoop;
    /**
     * 线程数量
     */
    int threadNum;

    int position;

} SubReactor;

SubReactor *initSubReactor();

NioEventLoop *selectSubReactorWorkEventLoop(SubReactor *subReactor);

#endif //IO_CONNECT_SUBREACTOR_H
