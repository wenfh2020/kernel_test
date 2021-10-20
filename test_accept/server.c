#include "server.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>

int g_epfd = -1;          /* epoll file descriptor. */
int g_listen_fd = -1;     /* listen socket's file descriptor. */
int g_ls_array[16] = {0}; /* listen socket's file descriptor's array. */

int init_server(const char *ip, int port) {
    LOG("init server.....");

    int reuse = 1;
    int listen_fd = -1;
    struct sockaddr_in sa;

    /* create socket. */
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        LOG_SYS_ERR("create socket failed!");
        return -1;
    }
    LOG("create listen socket, fd: %d.", listen_fd);

    /* bind. */
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr =
        (ip == NULL || !strlen(ip)) ? htonl(INADDR_ANY) : inet_addr(ip);

    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    if (bind(listen_fd, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
        LOG_SYS_ERR("bind failed! fd: %d", listen_fd);
        close(listen_fd);
        return -1;
    }

    /* listen */
    if (listen(listen_fd, BACKLOG) == -1) {
        LOG_SYS_ERR("listen failed!");
        close(listen_fd);
        return -1;
    }

    LOG("server start now, ip: %s, port: %d.",
        (ip == NULL || !strlen(ip)) ? "127.0.0.1" : ip, port);
    return listen_fd;
}

void run_server(int listen_fd) {
    LOG("run server.....");

    int fd = -1;
    int port = 0;
    struct sockaddr addr;
    struct sockaddr_in *sai;
    socklen_t addr_len = sizeof(addr);

    char cip[64] = {0};
    char buf[BUFFER_LEN] = {0};

    while (1) {
        /* get new client. */
        fd = accept(listen_fd, &addr, &addr_len);
        if (fd == -1) {
            LOG_SYS_ERR("accept failed!");
            break;
        }

        /* get client's connection info. */
        sai = (struct sockaddr_in *)&addr;
        inet_ntop(AF_INET, (void *)&(sai->sin_addr), cip, 32);
        port = ntohs(sai->sin_port);
        LOG("accept new client, pid: %d, fd: %d, ip: %s, port: %d",
            getpid(), fd, cip, port);

        /* recv data. */
        ssize_t nread = read(fd, buf, BUFFER_LEN);
        if (nread == -1) {
            LOG_SYS_ERR("read data failed! fd: %d!", fd);
            break;
        } else if (nread == 0) {
            close(fd);
            fd = -1;
            LOG("conn is closed! fd: %d", fd);
            continue;
        }
        LOG("read data: %s, len: %d, fd: %d", buf, nread, fd);

        /* send data. */
        ssize_t nwritten = write(fd, buf, nread);
        if (nwritten < 0) {
            LOG_SYS_ERR("send data failed! fd: %d!", fd);
            break;
        }
        LOG("send data: %s, len: %d, fd: %d", buf, nwritten, fd);
    }

    if (fd != -1) close(fd);
    if (listen_fd != -1) close(listen_fd);
    LOG("run server end!");
}