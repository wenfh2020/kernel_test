#include "server.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#define _EPOLL_EXCLUSIVE_

#ifdef _EPOLL_EXCLUSIVE_
#ifndef EPOLLEXCLUSIVE
/* Set exclusive wakeup mode for the target file descriptor */
#define EPOLLEXCLUSIVE (1U << 28)
#endif
#endif

int init_server(const char *ip, int port) {
    int s;
    int reuse = 1;
    struct sockaddr_in sa;

    /* create socket. */
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        LOG_SYS_ERR("create socket failed!");
        return -1;
    }
    LOG("create listen socket, fd: %d.", s);

    /* bind. */
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr =
        (ip == NULL || !strlen(ip)) ? htonl(INADDR_ANY) : inet_addr(ip);

    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    if (bind(s, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
        LOG_SYS_ERR("bind failed! fd: %d", s);
        close(s);
        return -1;
    }

    /* listen */
    if (listen(s, BACKLOG) == -1) {
        LOG_SYS_ERR("listen failed!");
        close(s);
        return -1;
    }

    LOG("set socket nonblocking. fd: %d.", s);

    /* set nonblock */
    if (set_nonblocking(s) < 0) {
        return -1;
    }

    LOG("server start now, ip: %s, port: %d.",
        (ip == NULL || !strlen(ip)) ? "127.0.0.1" : ip, port);
    return s;
}

void run_server(int worker_index, int listen_fd) {
    LOG("run server, worker index: %d, pid: %d", worker_index, getpid());

    char ip[32];
    char buf[BUFFER_LEN];
    int i, n, fd, cfd, epfd, port, len;
    struct epoll_event ee, *ees;

    struct sockaddr addr;
    struct sockaddr_in *sai;
    socklen_t addr_len = sizeof(addr);

    epfd = epoll_create(100);
    if (epfd < 0) {
        LOG_SYS_ERR("epoll create failed!");
        return;
    }
    LOG("create epoll fd: %d", epfd);

#ifdef _EPOLL_EXCLUSIVE_
    ee.events = EPOLLIN | EPOLLEXCLUSIVE;
#else
    ee.events = EPOLLIN;
#endif
    ee.data.fd = listen_fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ee) < 0) {
        LOG_SYS_ERR("epoll_ctl add event: <EPOLLIN> failed! fd: %d.", listen_fd);
        return;
    }
    LOG("epoll_ctl add event: <EPOLLIN>, epfd: %d, fd: %d.", epfd, listen_fd);

    ees = (struct epoll_event *)calloc(EVENTS_SIZE, sizeof(struct epoll_event));
    while (1) {
        n = epoll_wait(epfd, ees, EVENTS_SIZE, 500);
        for (i = 0; i < n; i++) {
            // LOG("xxxxxxxxxxxxxxxxxxxxxxxx");
            fd = ees[i].data.fd;
            if (fd == listen_fd) {
                /* sleep, in order to reproduce the thundering herd. */
                usleep(500);

                /* accept new client. */
                cfd = accept(listen_fd, &addr, &addr_len);
                if (cfd < 0) {
                    if (errno == EINTR || errno == EAGAIN) {
                        LOG("accept next time!");
                        continue;
                    } else {
                        LOG_SYS_ERR("accept failed!");
                        break;
                    }
                } else {
                    /* get client's connection info. */
                    sai = (struct sockaddr_in *)&addr;
                    inet_ntop(AF_INET, (void *)&(sai->sin_addr), ip, 32);
                    port = ntohs(sai->sin_port);
                    LOG("accept a new client, fd: %d, ip: %s, port: %d", cfd, ip, port);

                    /* add client event to epoll. */
                    ee.events = EPOLLIN;
                    ee.data.fd = cfd;
                    if (epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ee) < 0) {
                        LOG_SYS_ERR("epoll_ctl add event: <EPOLLIN> failed! fd: %d.", cfd);
                        return;
                    }
                    LOG("epoll_ctl add event: <EPOLLIN>, epfd: %d, fd: %d.", epfd, cfd);
                }
            } else {
                /* recv data. */
                len = read(cfd, buf, BUFFER_LEN);
                if (len == -1) {
                    LOG_SYS_ERR("read data failed! fd: %d!", cfd);
                    break;
                } else if (len == 0) {
                    close(cfd);
                    LOG("close client fd: %d", cfd);
                    cfd = -1;
                    continue;
                }
                LOG("read data: %s, len: %d, fd: %d", buf, len, cfd);

                /* send data. */
                len = write(cfd, buf, len);
                if (len < 0) {
                    LOG_SYS_ERR("send data failed! fd: %d!", cfd);
                    break;
                }
                LOG("send data: %s, len: %d, fd: %d", buf, len, cfd);
            }
        }
    }

    LOG("exit from pid: %d", getpid());
}
