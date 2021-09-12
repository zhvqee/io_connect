//
// Created by 朱琪 on 2021/9/11.
//

#include "subreactor.h"

static void *initWorkerEventLoop(void *args);


SubReactor *initSubReactor() {
    SubReactor *subReactor = malloc(sizeof(SubReactor));
    int enableCPUNum = sysconf(_SC_NPROCESSORS_ONLN);
    subReactor->threadNum = enableCPUNum;
    subReactor->position = 0;
    size_t size = sizeof(NioEventLoop *) * subReactor->threadNum;
    subReactor->nioEventLoop = (NioEventLoop **) malloc(size);

    for (int i = 0; i < subReactor->threadNum; i++) {
        subReactor->nioEventLoop[i] = NULL;
    }

    for (int i = 0; i < subReactor->threadNum; i++) {
        subReactor->nioEventLoop[i] = initNioEventLoop("worker-thread");
    }
    pthread_mutex_init(&subReactor->mutex, NULL);
    for (int i = 0; i < subReactor->threadNum; i++) {
        pthread_t pthread = 0;
        pthread_create(&pthread, NULL, initWorkerEventLoop, subReactor->nioEventLoop[i]);
    }
    return subReactor;
}

/**
* 异步线程执行
* @param args
* @return
*/
static void *initWorkerEventLoop(void *args) {
    NioEventLoop *nioEventLoop = (NioEventLoop *) args;
    nioEventLoop->ownerThreadId = pthread_self();
    nioEventLoopRun(nioEventLoop, 1);
}

/**
 * 非线程安全
 * @param subReactor
 * @return
 */
NioEventLoop *selectSubReactorWorkEventLoop(SubReactor *subReactor) {
    int pos = subReactor->position & (subReactor->threadNum - 1);
    subReactor->position = subReactor->position + 1;
    return subReactor->nioEventLoop[pos];
}

