//
// Created by zhuqi on 2021/9/9.
//

#include "event.h"
#include "assert.h"
#include "pthread.h"
#include "logs/log.h"
#include "global.h"


void nioChannelPendingHandlerAdd(struct NioEventLoop *nioEventLoop, struct Channel *pChannel);

/**
 * 在 业务处理完channel后，需要重新处理channel 链表
 * @param nioEventLoop
 */
void nioChannelPendingHandler(struct NioEventLoop *nioEventLoop) {
    pthread_mutex_lock(&nioEventLoop->mutex);

    struct ChannelNode *p = nioEventLoop->head;
    while (p) {
        if (p->channelType == 1) { //add
            nioChannelPendingHandlerAdd(nioEventLoop, p->data);

        } else if (p->channelType == 2) { //

            nioChannelPendingHandlerAdd(nioEventLoop, p->data);
        }
        p = p->next;
    }
    nioEventLoop->head = nioEventLoop->tail = NULL;
    pthread_mutex_unlock(&nioEventLoop->mutex);
}

void nioChannelPendingHandlerAdd(struct NioEventLoop *nioEventLoop, struct Channel *pChannel) {

}

void initEventDispatcher() {
    eventDispatcher = (struct EventDispatcher *)malloc(sizeof(struct EventDispatcher *));
    eventDispatcher->dispatch=dispatch();
}

void nioEventLoopInit(struct NioEventLoop *nioEventLoop) {
    nioEventLoop->size = 1000;
    nioEventLoop->head = nioEventLoop->tail = NULL;
    nioEventLoop->quit = 0;
    nioEventLoop->ownerThreadId = pthread_self();
    nioEventLoop->channelMap = channelMap;
    nioEventLoop->eventDispatcher = eventDispatcher;
}


void nioEventLoopRun(struct NioEventLoop *nioEventLoop) {
    assert(nioEventLoop);

    if (nioEventLoop->ownerThreadId != pthread_self()) {
        log(ERROR, "current nioEventLoopRun thread execute is not %ld ", nioEventLoop->ownerThreadId);
        return;
    }

    const struct EventDispatcher *eventDispatcher = nioEventLoop->eventDispatcher;

    struct timeval timeval;
    timeval.tv_sec = 1;
    while (nioEventLoop->quit != 0) {
        eventDispatcher->dispatch(nioEventLoop, &timeval);
        nioChannelPendingHandler(nioEventLoop);
    }
}
