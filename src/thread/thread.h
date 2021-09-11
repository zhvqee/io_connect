//
// Created by 朱琪 on 2021/9/11.
//

#ifndef IO_CONNECT_THREAD_H
#define IO_CONNECT_THREAD_H

#include "thread.h"

typedef struct Runnable {

    void *threadData;

    void *(*run)(void *);
}Runnable;

typedef struct ThreadLoop {

    // 当前线程没有任务时，阻塞，wait
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    char threadName[58];
    pthread_t threadId;        /* thread ID */

    /**
     * 运行的任务
     */
    struct Runnable *runTask;

} ThreadLoop;

typedef struct ThreadPool {
    int started;

    //线程数目
    int threadNumber;

    int position;

    char *threadGroupName;
    /**
     * 线程组
     */
    ThreadLoop *threadLoops;

} ThreadPool;

ThreadPool *beginThreadPool(int threadNumber, char *threadName);


#endif //IO_CONNECT_THREAD_H
