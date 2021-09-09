//
// Created by 朱琪 on 2021/9/8.
//

#ifndef IO_CONNECT_DEFAULT_H
#define IO_CONNECT_DEFAULT_H

void *defaultAfterAccept(socket_fd_t fd) {
   // log(INFO, "defaultAfterAccept method invoke current socket fd %d", fd);
    return 0;
}

#endif //IO_CONNECT_DEFAULT_H
