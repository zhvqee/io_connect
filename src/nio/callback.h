//
// Created by 朱琪 on 2021/9/8.
//

#ifndef IO_CONNECT_CALLBACK_H
#define IO_CONNECT_CALLBACK_H

typedef void *(*callback)(void *arg);

typedef int socket_fd_t;


/**
 *  服务端接受连接后，回调函数
 * @param cb
 * @param fd
 * @return
 */
void *afterAccept(callback cb, socket_fd_t fd);

/**
 *  服务端数据读取，即编码后 业务处理回调callback
 * @param fd
 * @param cb
 * @return
 */
void *afterRead(socket_fd_t fd, callback cb);

/**
 * 服务端数据 写支持处理，
 * @param fd
 * @param cb
 * @return
 */
void *beforeWrite(socket_fd_t fd, callback cb);


#endif //IO_CONNECT_CALLBACK_H
