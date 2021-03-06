//
// Created by zhuqi on 2021/9/9.
//

#include "event.h"
#include "assert.h"
#include "pthread.h"
#include "../../logs/log.h"
#include "../global/global.h"
#include <sys/socket.h>

static ChannelMap *initChannelMap();

static void *initEventDispatcher(NioEventLoop *nioEventLoop);

static void nioEventLoopDoChannelEvent(NioEventLoop *nioEventLoop,
                                       int fd,
                                       Channel *channel,
                                       int type);

static ChannelNode *initChannelNode(Channel *channel, int opType);

static void nioEventLoopHandleChannels(struct NioEventLoop *nioEventLoop);

static void nioChannelPendingHandlerAdd(struct NioEventLoop *nioEventLoop, struct Channel *channel);

static int makeSpaceChannelMap(struct ChannelMap *channelMap, int fd);

static void wakeupEventLoopThread(struct NioEventLoop *nioEventLoop);

static void wakeupEventLoopHandle(struct NioEventLoop *nioEventLoop);

NioEventLoop *initNioEventLoop(char *threadName) {
    NioEventLoop *nioEventLoop = (NioEventLoop *) malloc(sizeof(NioEventLoop));
    nioEventLoop->quit = 0;

    nioEventLoop->ownerThreadId = pthread_self();
    nioEventLoop->threadName = threadName;

    nioEventLoop->channelMap = initChannelMap();

    pthread_mutex_init(&nioEventLoop->mutex, NULL);
    pthread_cond_init(&nioEventLoop->condition, NULL);

    nioEventLoop->dispatchData = NULL;
    nioEventLoop->head = NULL;
    nioEventLoop->tail = NULL;

    nioEventLoop->eventDispatcher = initEventDispatcher(nioEventLoop);

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, nioEventLoop->socketPair) < 0) {
        io_log(ERROR, "socketpair set fialed");
    }
    Channel *channel = initChannel(nioEventLoop->socketPair[1], NIO_EVENT_READ, wakeupEventLoopHandle, NULL,
                                   nioEventLoop);
    addNioEventLoopChannelEvent(nioEventLoop, channel);
    return nioEventLoop;
}

void addNioEventLoopChannelEvent(NioEventLoop *nioEventLoop, Channel *channel) {
    nioEventLoopDoChannelEvent(nioEventLoop, channel->fd, channel, 1);
}

Channel *initChannel(int fd,
                     int events,
                     CallbackReadHandler callbackReadHandler,
                     CallbackWriteHandler callbackWriteHandler,
                     void *data) {
    Channel *channel = (Channel *) malloc(sizeof(Channel));
    channel->fd = fd;
    channel->events = events;
    channel->readHandler = callbackReadHandler;
    channel->writeHandler = callbackWriteHandler;
    channel->channelData = data;
    return channel;
}

void channelEventActivate(NioEventLoop *nioEventLoop, int fd, int events) {
    ChannelMap *channelMap = nioEventLoop->channelMap;
    if (fd < 0) {
        return;
    }
    if (fd >= channelMap->size) {
        return;
    }
    Channel *channel = channelMap->channels[fd];

    assert(channel->fd == fd);

    if (events & (NIO_EVENT_READ)) {
        if (channel->readHandler) {
            channel->readHandler(channel->channelData);
        }
    }
    if (events & (NIO_EVENT_WRITE)) {
        if (channel->writeHandler) {
            channel->writeHandler(channel->channelData);
        }
    }
}

/**
 *
 * ???????????? ?????????channel ????????????acceptor ??????????????????
 * 1) ????????? listenfd  ???????????? acceptor ?????? eventdispatcher
 * 2) ????????? clientId ?????????nioEventLoop ?????????worker eventLoop, ?????????eventLoop ????????????accetor ?????????
 *   2.1 ??????????????????????????????dispatch ??????????????????????????????????????????????????????
 *
 * @param nioEventLoop
 * @param fd
 * @param channel
 * @param type
 */
static void nioEventLoopDoChannelEvent(NioEventLoop *nioEventLoop,
                                       int fd,
                                       Channel *channel,
                                       int type) {
    //get the lock
    pthread_mutex_lock(&nioEventLoop->mutex);

    //??????????????????
    ChannelNode *node = nioEventLoop->tail;
    ChannelNode *insertNode = initChannelNode(channel, type);
    nioEventLoop->tail = insertNode;

    if (node) {
        node->next = insertNode;
    } else {
        nioEventLoop->head = insertNode;
    }
    //release the lock
    pthread_mutex_unlock(&nioEventLoop->mutex);


    if (nioEventLoop->ownerThreadId != pthread_self()) {
        wakeupEventLoopThread(nioEventLoop);
    } else {
        nioEventLoopHandleChannels(nioEventLoop);
    }
}

/**
 * ??????????????????????????? ????????? epoll??????fd?????????epoll????????????????????????????????????????????????
 */
static void wakeupEventLoopThread(struct NioEventLoop *nioEventLoop) {
    char one = 'a';
    ssize_t n = write(nioEventLoop->socketPair[0], &one, sizeof(one));
    if (n != sizeof(one)) {
        io_log(ERROR, "wakeup event loop thread failed");
    }
}

static void wakeupEventLoopHandle(struct NioEventLoop *nioEventLoop) {
    char one = 'a';
    ssize_t n = read(nioEventLoop->socketPair[1], &one, sizeof(one));
    if (n != sizeof(one)) {
        io_log(ERROR, "read wakeup event loop thread failed");
    }
}


static ChannelMap *initChannelMap() {
    ChannelMap *channelMap = (ChannelMap *) malloc(sizeof(ChannelMap));
    channelMap->channels = NULL;
    channelMap->size = 0;
    return channelMap;
}

static ChannelNode *initChannelNode(Channel *channel, int opType) {
    ChannelNode *channelNode = (ChannelNode *) malloc(sizeof(struct ChannelNode));
    channelNode->data = channel;
    channelNode->opType = opType;
    channelNode->next = NULL;
    return channelNode;
}


static void *initEventDispatcher(NioEventLoop *nioEventLoop) {
    // ????????????
    const EventDispatcher *eventDispatcher = &kqueueDispatcher;

    nioEventLoop->eventDispatcher = eventDispatcher;

    //???????????????????????????????????? kqueue,?????????????????????????????????????????????kqueue descriptor
    nioEventLoop->dispatchData = eventDispatcher->init(nioEventLoop);

    return eventDispatcher;
}

/**
 * io ??????????????????NioEventLoop ?????????io ????????????
 * ??? ???????????????channel????????????????????????channel ??????
 * @param nioEventLoop
 */
static void nioEventLoopHandleChannels(struct NioEventLoop *nioEventLoop) {
    pthread_mutex_lock(&nioEventLoop->mutex);

    //????????????
    struct ChannelNode *p = nioEventLoop->head;
    while (p) {
        if (p->opType == 1) { //add
            nioChannelPendingHandlerAdd(nioEventLoop, p->data);

        } else if (p->opType == 2) { //??????

        } else if (p->opType == 3) { //??????


        }
        p = p->next;
    }
    nioEventLoop->head = nioEventLoop->tail = NULL;

    pthread_mutex_unlock(&nioEventLoop->mutex);
}

// ??????channel ??? event loop ???event dispatch,??????????????????
static void nioChannelPendingHandlerAdd(struct NioEventLoop *nioEventLoop, struct Channel *channel) {
    ChannelMap *channelMap = nioEventLoop->channelMap;
    struct EventDispatcher *eventDispatcher = nioEventLoop->eventDispatcher;
    int fd = channel->fd;
    if (fd > channelMap->size) {
        makeSpaceChannelMap(channelMap, fd);
    }

    if (channelMap->channels[fd] == NULL) {
        channelMap->channels[fd] = channel;
        eventDispatcher->add(nioEventLoop, channel);
    }
}

static int makeSpaceChannelMap(struct ChannelMap *channelMap, int fd) {
    if (channelMap->size > fd) {
        return 0;
    }
    int size = channelMap->size == 0 ? 32 : channelMap->size;
    while (size <= fd) {
        size <<= 1;
    }
    struct Channel **tmp = (Channel **) realloc(channelMap->channels, size * sizeof(struct Channel *));
    if (tmp == NULL)
        return (-1);

    memset(&tmp[channelMap->size], 0, (size - channelMap->size) * sizeof(struct Channel *));
    channelMap->size = size;
    channelMap->channels = tmp;
    return 0;

}


void nioEventLoopRun(struct NioEventLoop *nioEventLoop, int timed) {
    assert(nioEventLoop);

    if (nioEventLoop->ownerThreadId != pthread_self()) {
        io_log(ERROR, "current nioEventLoopRun thread execute is not %ld ", nioEventLoop->ownerThreadId);
        return;
    }

    const struct EventDispatcher *eventDispatcher = nioEventLoop->eventDispatcher;

    struct timeval timeval;
    timeval.tv_sec = 1;
    while (nioEventLoop->quit == 0) {
        eventDispatcher->dispatch(nioEventLoop, timed ? &timeval : NULL);
        nioEventLoopHandleChannels(nioEventLoop);
    }
}
