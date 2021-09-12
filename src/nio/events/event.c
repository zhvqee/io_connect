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
 * 这个方法 在添加channel 时，是在acceptor 线上上调用，
 * 1) 如果是 listenfd  刚好是在 acceptor 上的 eventdispatcher
 * 2) 如果是 clientId 则调用nioEventLoop 是某个worker eventLoop, 而这个eventLoop 线程不是accetor 线程。
 *   2.1 所以可能该线程阻塞在dispatch 上，或者在执行其他任务，都需要唤醒它
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

    //添加到链表中
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
 * 随便发送一个字符给 阻塞在 epoll上的fd，那么epoll函数必定返回。但是需要去消费这个
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
    // 外部定义
    const EventDispatcher *eventDispatcher = &kqueueDispatcher;

    nioEventLoop->eventDispatcher = eventDispatcher;

    //初始化得到数据，比如调用 kqueue,返回一个自定义的结构，里面宝库kqueue descriptor
    nioEventLoop->dispatchData = eventDispatcher->init(nioEventLoop);

    return eventDispatcher;
}

/**
 * io 线程执行，是NioEventLoop 自己的io 线程执行
 * 在 业务处理完channel后，需要重新处理channel 链表
 * @param nioEventLoop
 */
static void nioEventLoopHandleChannels(struct NioEventLoop *nioEventLoop) {
    pthread_mutex_lock(&nioEventLoop->mutex);

    //循环处理
    struct ChannelNode *p = nioEventLoop->head;
    while (p) {
        if (p->opType == 1) { //add
            nioChannelPendingHandlerAdd(nioEventLoop, p->data);

        } else if (p->opType == 2) { //删除

        } else if (p->opType == 3) { //更新


        }
        p = p->next;
    }
    nioEventLoop->head = nioEventLoop->tail = NULL;

    pthread_mutex_unlock(&nioEventLoop->mutex);
}

// 添加channel 到 event loop 的event dispatch,用于多路复用
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
