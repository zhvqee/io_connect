//
// Created by 朱琪 on 2021/9/8.
//

#include "callback.h"
#include "../logs/log.h"

void *afterAccept(callback cb, socket_fd_t fd) {
    io_log(INFO, "afterAccept method invoke");
    if (cb) {
        return cb((void *) fd);
    }
}



void *afterRead(socket_fd_t fd, callback cb) {
    // 读取fd
    void *arg = NULL;

    return cb(arg);
}












