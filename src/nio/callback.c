//
// Created by 朱琪 on 2021/9/8.
//

#include "callback.h"
#include "../logs/log.h"
#include "default.h"

void *afterAccept(callback cb, socket_fd_t fd) {
    log(INFO, "afterAccept method invoke");
    if (cb) {
        return cb((void *) fd);
    }
    return defaultAfterAccept(fd);
}



void *afterRead(socket_fd_t fd, callback cb) {
    // 读取fd
    void *arg = NULL;

    return cb(arg);
}












