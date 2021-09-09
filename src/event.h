//
// Created by zhuqi on 2021/9/9.
//

#ifndef IO_CONNECT_EVENT_H
#define IO_CONNECT_EVENT_H

#include <pthread.h>
#include <time.h>

#define  NIO_EVENT_READ  1
#define  NIO_EVENT_WRITE 2


typedef int (*callbackWriteHandler)(void *);

typedef int (*callbackReadHandler)(void *);

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
    callbackReadHandler readHandler;

    /**
     * 写回调
     */
    callbackReadHandler writeHandler;

} Channel;

typedef struct ChannelMap {

    void *channels;

    int size;

} ChannelMap;

struct ChannelNode {
    int channelType;
    struct Channel *data;
    struct ChannelNode *next;
};

//向前申明
struct NioEventLoop;

/**
 * 事件派发器
 */
struct EventDispatcher {

    /**
     * 派发器的实现名，可以指定
     */
    const char *name;

    /**
     * 初始化
     * @param eventLoop
     */
    void (*init)(struct NioEventLoop *eventLoop);

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
    void (*dispatch)(struct NioEventLoop *eventLoop, struct timeval *timeval);

};


/**
 * IO线程执行loop, 线程ownerThreadId 处理链表head,到tail的内容
 */
struct NioEventLoop {
    int size;
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
    void *data;


    //channel list
    struct ChannelNode *head;

    struct ChannelNode *tail;

    /**
     * 事件派发器，channel 里的事件通过 eventDispatcher派发处理
     */
    const struct EventDispatcher *eventDispatcher;

};


#endif //IO_CONNECT_EVENT_H
