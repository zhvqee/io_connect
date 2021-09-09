//
// Created by zhuqi on 2021/9/9.
//

#include "event.h"
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include "stdlib.h"


struct KqueueData {
    int fd;
    struct kevent *kevent;
};

struct ChannelNode *newChannel(int fd, int mask) {
    struct ChannelNode *node = (struct ChannelNode *) malloc(sizeof(struct ChannelNode *));
    node->data = (struct Channel *) malloc(sizeof(struct Channel *));
    node->next = NULL;
    node->data->fd = fd;
    node->data->events = mask;
}


/**
    * 初始化
    * @param eventLoop
    */
void init(struct NioEventLoop *eventLoop) {
    int fd = kqueue();
    eventLoop->data = NULL;


}

/**
 * 添加一个channel
 * @param eventLoop
 * @param channel
 */
void (*add)(struct NioEventLoop *eventLoop, struct Channel *channel);

/**
 * 添加一个channel
 * @param eventLoop
  * @param channel
*/
void (*del)(struct NioEventLoop *eventLoop, struct Channel *channel);


/**
* 添加一个channel
* @param eventLoop
* @param channel
*/
void (*update)(struct NioEventLoop *eventLoop, struct Channel *channel);

/**
 * 清除
 * @param eventLoop
 */
void (*clear)(struct NioEventLoop *eventLoop);

int dispatch(struct NioEventLoop *eventLoop, struct timeval *timeval) {
    struct KqueueData *kqueueData = eventLoop->data;
    int retVal = 0;
    if (timeval) {

        struct timespec timespec;
        timespec.tv_sec = timeval->tv_sec;
        timespec.tv_nsec = timeval->tv_usec * 1000;
        retVal = kevent(kqueueData->fd, NULL, 0, kqueueData->kevent, eventLoop->size, &timespec);
    } else {
        retVal = kevent(kqueueData->fd, NULL, 0, kqueueData->kevent, eventLoop->size, NULL);
    }

    struct ChannelNode *tail = eventLoop->tail;

    if (retVal > 0) {
        for (int i = 0; i < retVal; i++) {
            int mask = 0;
            struct kevent *kevent = kqueueData->kevent + i;
            if (kevent->filter == EVFILT_READ) {
                mask |= NIO_EVENT_READ;
            }
            if (kevent->filter == EVFILT_WRITE) {
                mask |= NIO_EVENT_WRITE;
            }
            kevent->ident;
            struct ChannelNode *p = newChannel(kevent->ident, mask);
            if (tail == NULL) {
                tail = p;
            } else {
                tail->next = p;
                tail = p;
            }
        }
    }
    return retVal;

}



