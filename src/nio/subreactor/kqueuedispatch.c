//
// Created by zhuqi on 2021/9/9.
//

#include <stdio.h>
#include "../events/event.h"
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include "stdlib.h"
#include "../global/global.h"
#include "../events/event.h"
#include "unistd.h"

static void *kqueueInit(struct NioEventLoop *eventLoop);

void kqueueAdd(struct NioEventLoop *eventLoop, struct Channel *channel);

void kqueueDel(struct NioEventLoop *eventLoop, struct Channel *channel);

void kqueueUpdate(struct NioEventLoop *eventLoop, struct Channel *channel);

void kqueueClear(struct NioEventLoop *eventLoop);

int kqueueDispatch(struct NioEventLoop *eventLoop, struct timeval *timeval);

const struct EventDispatcher kqueueDispatcher = {
        "kqueue",
        kqueueInit,
        kqueueAdd,
        kqueueDel,
        kqueueUpdate,
        kqueueClear,
        kqueueDispatch,
};


typedef struct KqueueData {
    int kfd;
    struct kevent *listenEvent;
    struct kevent *triggerEvent;
    int size;
    int position;
} KqueueData;

/**
    * 初始化
    * @param eventLoop
    */
void *kqueueInit(struct NioEventLoop *eventLoop) {
    int fd = kqueue();
    struct KqueueData *kqueueData = (KqueueData *) malloc(sizeof(struct KqueueData));
    kqueueData->kfd = fd;

    kqueueData->triggerEvent = calloc(128, sizeof(struct kevent));
    kqueueData->listenEvent = calloc(128, sizeof(struct kevent));

    kqueueData->size = 128;
    kqueueData->position = 0;
    return kqueueData;
}

/**
 * 把当前的channel 填到 kqueue
 * @param eventLoop
 * @param channel
 */
void kqueueAdd(struct NioEventLoop *eventLoop, struct Channel *channel) {
    KqueueData *kqueueData = eventLoop->dispatchData;
    if (kqueueData->position >= kqueueData->size) {
        return;
    }
    struct kevent *target = &kqueueData->listenEvent[kqueueData->position];
    kqueueData->position++;
    if (channel->events & NIO_EVENT_READ) {
        EV_SET(target, channel->fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);

    }
    if (channel->events & NIO_EVENT_WRITE) {
        EV_SET(target, channel->fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, 0);
    }

}


/**
 * 添加一个channel
 * @param eventLoop
  * @param channel
*/
void kqueueDel(struct NioEventLoop *eventLoop, struct Channel *channel) {

}


/**
* 添加一个channel
* @param eventLoop
* @param channel
*/
void kqueueUpdate(struct NioEventLoop *eventLoop, struct Channel *channel) {

}

/**
 * 清除
 * @param eventLoop
 */
void kqueueClear(struct NioEventLoop *eventLoop) {

}


int kqueueDispatch(struct NioEventLoop *eventLoop, struct timeval *timeval) {
    struct KqueueData *kqueueData = eventLoop->dispatchData;
    int retVal = 0;

    if (timeval) {
        struct timespec timespec;
        timespec.tv_sec = timeval->tv_sec;
        timespec.tv_nsec = timeval->tv_usec * 1000;
        retVal = kevent(kqueueData->kfd, kqueueData->listenEvent, kqueueData->position, kqueueData->triggerEvent,
                        kqueueData->size, &timespec);
    } else {
        retVal = kevent(kqueueData->kfd, kqueueData->listenEvent, kqueueData->position, kqueueData->triggerEvent,
                        kqueueData->size, NULL);
    }


    struct kevent *e = kqueueData->triggerEvent;

    for (int i = 0; i < retVal; i++) {

        if (e[i].filter == EVFILT_EXCEPT) {
            fprintf(stderr, "epoll error\n");
            close(kqueueData->triggerEvent[i].ident);
            continue;
        }

        if (e[i].filter == EVFILT_READ) {
            channelEventActivate(eventLoop, e[i].ident, NIO_EVENT_READ);
        }
        if (e[i].filter == EVFILT_WRITE) {
            channelEventActivate(eventLoop, e[i].ident, NIO_EVENT_WRITE);
        }
    }

    return retVal;

}



