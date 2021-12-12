/* author: wenfh2020/2021.06.18
 * desc:   test tcp thundering herd in epoll.
 * ver:    Linux 5.0.1
 * test:   make rootfs --> input 's' --> input 'c' */

#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "server.h"

#define WORKER_CNT 2       /* fork workers to listen. */
#define SEND_DATA "hello." /* client send to server's test data. */

#define SERVER_PORT 5001      /* server's listen port. */
#define SERVER_IP "127.0.0.1" /* server's ip. */
// #define SERVER_IP "192.168.10.199" /* server's ip. */

/* fork a child process to run epoll server. */
void proc(const char *ip, int port);

/* fork workers to listen. */
int workers(int worker_cnt, const char *ip, int port);

int main(int argc, char **argv) {
    char buf[64] = {0};
    int port = SERVER_PORT;
    const char *ip = SERVER_IP;

    if (argc >= 3) {
        ip = argv[1];
        port = atoi(argv[2]);
    }

    LOG("pls input 's' to run server or 'c' to run client!");

    while (1) {
        scanf("%s", buf);

        if (strcmp(buf, "s") == 0) {
            proc(ip, port);
        } else if (strcmp(buf, "c") == 0) {
            proc_client(ip, port, SEND_DATA);
        } else {
            LOG("pls input 's' to run server or 'c' to run client!");
        }
    }

    return 0;
}

void proc(const char *ip, int port) {
    int pid;
    bring_up_net_interface(ip);

    pid = fork();
    if (pid == 0) {
        /* child */
        workers(WORKER_CNT, ip, port);
    } else if (pid > 0) {
        /* parent */
        LOG("pls input 'c' to run client!");
    } else {
        /* error */
        LOG_SYS_ERR("fork failed!");
        exit(-1);
    }
}

int workers(int worker_cnt, const char *ip, int port) {
    LOG("workers...");

    int i, fd, pid;

    fd = init_server(ip, port);
    if (fd < 0) {
        LOG("init server failed!");
        return 0;
    }

    for (i = 0; i < worker_cnt; i++) {
        pid = fork();
        if (pid == 0) {
            /* child. */
            run_server(i, fd);
        } else if (pid > 0) {
            /* parent */
            LOG("fork child, pid: %d", pid);
        } else {
            /* error */
            LOG_SYS_ERR("fork failed!");
            exit(-1);
        }
    }

    return 1;
}
