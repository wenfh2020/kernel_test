/* author: wenfh2020/2021.06.18
 * desc:   epoll test code, test tcp ipv4 async's server. 
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

#define SEND_DATA "hello." /* client send to server's test data. */

#define SERVER_PORT 5001      /* server's listen port. */
#define SERVER_IP "127.0.0.1" /* server's ip. */

/* fork a child process to run epoll server. */
void proc(const char *ip, int port);

/* run server, read data and send back to client. */
int proc_server(const char *ip, int port);

/* client send 'hello' to server. */
int proc_client(const char *ip, int port, char *data);

int main(int argc, char **argv) {
    char buf[64] = {0};
    int port = SERVER_PORT;
    const char *ip = SERVER_IP;

    if (argc >= 3) {
        ip = argv[1];
        port = atoi(argv[2]);
    }

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
    int pid = fork();
    if (pid == 0) {
        /* child */
        proc_server(ip, port);
    } else if (pid > 0) {
        /* parent */
        LOGC("pls input 'c' to run client!");
    } else {
        /* error */
        LOG_SYS_ERR("fork failed!");
        exit(-1);
    }
}

int proc_server(const char *ip, int port) {
    LOG("run server...");

    bring_up_net_interface(ip);
    if (init_server(ip, port) < 0) {
        LOG("init server failed!");
        return 0;
    }
    run_server();
    return 1;
}

int proc_client(const char *ip, int port, char *data) {
    LOGC("run client...");

    int fd, ret;
    char buf[1024];
    char portstr[8];
    struct sockaddr_in addr;

    strcpy(buf, data);
    snprintf(portstr, 8, "%d", port);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    memset(&addr.sin_zero, 0, 8);

    fd = socket(PF_INET, SOCK_STREAM, AF_UNSPEC);
    if (fd < 0) {
        LOGC_SYS_ERR("create new socket failed!");
        return 0;
    }

    ret = connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr));
    if (ret != 0) {
        LOGC_SYS_ERR("connect failed!");
        close(fd);
        return 0;
    }

    LOGC("client connect to server! ip: %s, port: %d.", ip, port);

    ret = write(fd, buf, strlen(buf));
    if (ret < 0) {
        LOGC_SYS_ERR("send failed! fd: %d", fd);
        return 0;
    }
    LOGC("send data to server, data: %s", buf);

    ret = read(fd, buf, sizeof(buf));
    if (ret <= 0) {
        LOGC_SYS_ERR("read data failed! fd: %d.", fd);
        return 0;
    }
    LOGC("read data from server, data: %s", buf);

    close(fd);
    return 1;
}
