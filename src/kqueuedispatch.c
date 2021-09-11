//
// Created by zhuqi on 2021/9/9.
//

#include <stdio.h>
#include "nio/event.h"
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include "stdlib.h"
#include "nio/global.h"
#include "nio/event.h"
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
        kqueueDispatch,
        kqueueClear,
};


typedef struct KqueueData {
    int fd;
    struct kevent *kevent;
    int size;
} KqueueData;

/**
    * 初始化
    * @param eventLoop
    */
void *kqueueInit(struct NioEventLoop *eventLoop) {
    int fd = kqueue();
    struct KqueueData *kqueueData = (KqueueData *) malloc(sizeof(struct KqueueData));
    kqueueData->fd = fd;
    kqueueData->kevent = calloc(128, sizeof(struct kevent));
    kqueueData->size = 128;
    return kqueueData;
}


void kqueueAdd(struct NioEventLoop *eventLoop, struct Channel *channel) {
    KqueueData *kqueueData = eventLoop->dispatchData;

    if (channel->events & NIO_EVENT_READ) {
        EV_SET(kqueueData->kevent, kqueueData->fd, EVFILT_READ, EV_ADD, 0, 0, NULL);

    }
    if (channel->events & NIO_EVENT_WRITE) {
        EV_SET(kqueueData->kevent, kqueueData->fd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
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
        retVal = kevent(kqueueData->fd, NULL, 0, kqueueData->kevent, kqueueData->size, &timespec);
    } else {
        retVal = kevent(kqueueData->fd, NULL, 0, kqueueData->kevent, kqueueData->size, NULL);
    }


    struct kevent *e = kqueueData->kevent;

    for (int i = 0; i < retVal; i++) {

        if (e[i].filter == EVFILT_EXCEPT) {
            fprintf(stderr, "epoll error\n");
            close(kqueueData->kevent[i].ident);
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



