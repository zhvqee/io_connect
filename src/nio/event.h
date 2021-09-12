//
// Created by zhuqi on 2021/9/9.
//

#ifndef IO_CONNECT_EVENT_H
#define IO_CONNECT_EVENT_H

#include <pthread.h>
#include <sys/time.h>

#define  NIO_EVENT_READ  1
#define  NIO_EVENT_WRITE 2

extern const struct EventDispatcher kqueueDispatcher;


typedef int (*CallbackWriteHandler)(void *);

typedef int (*CallbackReadHandler)(void *);


typedef struct Channel {
    /**
     * 描述符
     */
    int fd;

    /**
     * 事件类型
     */
    int events;


    /**
   * 参数的数据
   */
    void *channelData;

    /**
     * 读回调
     */
    CallbackWriteHandler readHandler;

    /**
     * 写回调
     */
    CallbackWriteHandler writeHandler;

} Channel;

typedef struct ChannelMap {

    struct Channel **channels;

    int size;

} ChannelMap;

typedef struct ChannelNode {
    int opType;
    struct Channel *data;
    struct ChannelNode *next;
} ChannelNode;

//向前申明
struct NioEventLoop;

/**
 * 事件派发器
 */
typedef struct EventDispatcher {

    /**
     * 派发器的实现名，可以指定
     */
    const char *name;

    /**
     * 初始化
     * @param eventLoop
     */
    void *(*init)(struct NioEventLoop *eventLoop);

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

    /**
     * 派发事件， 如果有IO事件就进行派发处理，否则阻塞
     * @param eventLoop
     */
    int (*dispatch)(struct NioEventLoop *eventLoop, struct timeval *timeval);

} EventDispatcher;


/**
 * IO线程执行loop, 线程ownerThreadId 处理链表head,到tail的内容
 */
typedef struct NioEventLoop {
    /**
     * 是否停止
     */
    int quit;

    /**
     * 拥有的线程ID
     */
    pthread_t ownerThreadId;

    /**
     * 线程名称
     */
    char *threadName;

    /**
     * 线程条件变量
     */
    pthread_cond_t condition;

    /**
     * 互斥量
     */
    pthread_mutex_t mutex;


    /**
     * channel map
     */
    ChannelMap *channelMap;

    /**
   * 参数的数据
   */
    void *dispatchData;


    //channel list
    struct ChannelNode *head;

    struct ChannelNode *tail;

    /**
     * 事件派发器，channel 里的事件通过 eventDispatcher派发处理
     */
    const struct EventDispatcher *eventDispatcher;

    int socketPair[2];

} NioEventLoop;


/**
 * 初始化init
 * @param threadName
 * @return
 */
NioEventLoop *initNioEventLoop(char *threadName);

/**
 * 添加一个channel 到eventloop
 * @param nioEventLoop
 * @param channel
 */
void addNioEventLoopChannelEvent(struct NioEventLoop *nioEventLoop, Channel *channel);

/**
 * 创建一个channel ，通过 socket fd ，主要进行封装
 * @param fd
 * @param events
 * @param callbackReadHandler
 * @param callbackWriteHandler
 * @param data
 * @return
 */
Channel *initChannel(int fd,
                     int events,
                     CallbackReadHandler callbackReadHandler,
                     CallbackWriteHandler callbackWriteHandler,
                     void *data);

/**
 * 当前fd 套接字触发 可读，可写时间
 * @param nioEventLoop
 * @param fd
 * @param events
 */
void channelEventActivate(NioEventLoop *nioEventLoop, int fd, int events);

void nioEventLoopRun(struct NioEventLoop *nioEventLoop, int timed);

#endif //IO_CONNECT_EVENT_H
