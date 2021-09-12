//
// Created by 朱琪 on 2021/9/11.
//

#include <stdlib.h>
#include <pthread.h>
#include "thread.h"
#include "stdio.h"
#include "assert.h"
#include "strings.h"


static ThreadPool *createThreadPool(int threadNumber, char *threadName);

static void *threadRunTask(void *args);

static void initThreadLoop(char *threadName, struct ThreadLoop *threadLoop, int idx);

static void startThreadLoop(struct ThreadLoop *threadLoop);

static void startThreadPool(ThreadPool *threadPool);


ThreadPool *beginThreadPool(int threadNumber, char *threadName) {
    ThreadPool *threadPool = createThreadPool(threadNumber, threadName);
    startThreadPool(threadPool);
    return threadPool;
}

void submit(Runnable *runnable) {

}


static ThreadPool *createThreadPool(int threadNumber, char *threadName) {
    struct ThreadPool *threadPool = (ThreadPool *) malloc(sizeof(ThreadPool));
    threadPool->position = 0;
    threadPool->started = 0;
    threadPool->threadNumber = threadNumber;
    threadPool->threadGroupName = threadName;
    threadPool->threadLoops = NULL;
    threadPool->submit = submit;
    return threadPool;
}


static void *threadRunTask(void *args) {
    struct ThreadLoop *threadLoop = (ThreadLoop *) args;
    assert(pthread_mutex_lock(&threadLoop->mutex) == 0);

    // 保证线程不结束
    while (1) {
        // 当前线程没有任务时，阻塞,在其他地方 唤起线程
        while (threadLoop->runTask == NULL) {
            assert(pthread_cond_wait(&threadLoop->cond, &threadLoop->mutex) == 0);
        }
        assert(pthread_mutex_unlock(&threadLoop->mutex) == 0);
        threadLoop->runTask->run(threadLoop->runTask->threadData);
    }
}

static void initThreadLoop(char *threadName, struct ThreadLoop *threadLoop, int idx) {

    pthread_mutex_init(&threadLoop->mutex, NULL);
    pthread_cond_init(&threadLoop->cond, NULL);
    threadLoop->runTask = NULL;
    threadLoop->threadId = 0;
    strcpy(threadLoop->threadName, threadName);
    sprintf(threadLoop->threadName + strlen(threadName), "%d", idx);
}

static void startThreadLoop(struct ThreadLoop *threadLoop) {
    pthread_create(&threadLoop->threadId, NULL, &threadRunTask, threadLoop);
}

static void startThreadPool(ThreadPool *threadPool) {
    assert(threadPool->started == 0);
    threadPool->started = 1;
    if (threadPool->threadNumber <= 0) {
        return;
    }
    threadPool->threadLoops = malloc(sizeof(struct ThreadLoop) * threadPool->threadNumber);
    for (int i = 0; i < threadPool->threadNumber; i++) {
        initThreadLoop(threadPool->threadGroupName, &threadPool->threadLoops[i], i);
        startThreadLoop(&threadPool->threadLoops[i]);
    }
}

