#include "common.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <net/if.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>

#define MAX_IFS 64
#define LOOP_IP "127.0.0.1"

int log_data(int is_err, const char *file,
             const char *func, int file_line, int err, const char *fmt, ...) {
    int off;
    va_list ap;
    char buf[64];
    char msg[1024];
    struct timeval tv;
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    gettimeofday(&tv, NULL);
    off = strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S.", tm);
    snprintf(buf + off, sizeof(buf) - off, "%03d", (int)tv.tv_usec / 1000);

    if (is_err) {
        printf("[%d][%s][%s][%s:%d][errno: %d, errstr: %s] %s\n",
               getpid(), buf, file, func, file_line, err, strerror(err), msg);
    } else {
        printf("[%d][%s][%s][%s:%d] %s\n",
               getpid(), buf, file, func, file_line, msg);
    }

    return 1;
}

int bring_up_net_interface(const char *ip) {
    int fd;
    struct ifreq ifreqlo;
    struct sockaddr_in sa;

    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr(LOOP_IP);
    fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

    strncpy(ifreqlo.ifr_name, "lo", sizeof("lo"));
    memcpy((char *)&ifreqlo.ifr_addr, (char *)&sa, sizeof(struct sockaddr));
    ioctl(fd, SIOCSIFADDR, &ifreqlo);
    ioctl(fd, SIOCGIFFLAGS, &ifreqlo);
    ifreqlo.ifr_flags |= IFF_UP | IFF_LOOPBACK | IFF_RUNNING;
    ioctl(fd, SIOCSIFFLAGS, &ifreqlo);
    close(fd);

    if (ip != NULL && strlen(ip) != 0 && strcmp(ip, LOOP_IP) != 0) {
        LOG("bring up interface: eth0, ip: %s", ip);
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = inet_addr(ip);
        fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
        strncpy(ifreqlo.ifr_name, "eth0", sizeof("eth0"));
        memcpy((char *)&ifreqlo.ifr_addr, (char *)&sa, sizeof(struct sockaddr));
        ioctl(fd, SIOCSIFADDR, &ifreqlo);
        ioctl(fd, SIOCGIFFLAGS, &ifreqlo);
        ifreqlo.ifr_flags |= IFF_UP | IFF_RUNNING;
        ((unsigned char *)&ifreqlo.ifr_hwaddr.sa_data)[0] = 0x02;
        ((unsigned char *)&ifreqlo.ifr_hwaddr.sa_data)[1] = 0x42;
        ((unsigned char *)&ifreqlo.ifr_hwaddr.sa_data)[2] = 0xc0;
        ((unsigned char *)&ifreqlo.ifr_hwaddr.sa_data)[3] = 0xa8;
        ((unsigned char *)&ifreqlo.ifr_hwaddr.sa_data)[4] = 0x28;
        ((unsigned char *)&ifreqlo.ifr_hwaddr.sa_data)[5] = 0x05;
        if (ioctl(fd, SIOCSIFFLAGS, &ifreqlo) < 0) {
            LOG_SYS_ERR("ioctl(SIOCGIFCONF) failed!");
            return 0;
        }
        close(fd);
    }

    LOG("list all interfaces:");

    int s;
    struct ifreq *ifr, *ifend;
    struct ifreq ifreq;
    struct ifconf ifc;
    struct ifreq ifs[MAX_IFS];

    s = socket(PF_INET, SOCK_DGRAM, 0);

    ifc.ifc_len = sizeof(ifs);
    ifc.ifc_req = ifs;
    if (ioctl(s, SIOCGIFCONF, &ifc) < 0) {
        LOG_SYS_ERR("ioctl(SIOCGIFCONF) failed!");
        return 0;
    }

    ifend = ifs + (ifc.ifc_len / sizeof(struct ifreq));
    for (ifr = ifc.ifc_req; ifr < ifend; ifr++) {
        LOG("interface: %s", ifr->ifr_name);

        if (ifr->ifr_addr.sa_family == AF_INET) {
            strncpy(ifreq.ifr_name, ifr->ifr_name, sizeof(ifreq.ifr_name));
            if (ioctl(s, SIOCGIFHWADDR, &ifreq) < 0) {
                LOG_SYS_ERR("SIOCGIFHWADDR(%s): %m", ifreq.ifr_name);
                return 0;
            }

            LOG("ip address: %s",
                inet_ntoa(((struct sockaddr_in *)&ifr->ifr_addr)->sin_addr));
            LOG("device: %s -> Ethernet %02x:%02x:%02x:%02x:%02x:%02x", ifreq.ifr_name,
                (int)((unsigned char *)&ifreq.ifr_hwaddr.sa_data)[0],
                (int)((unsigned char *)&ifreq.ifr_hwaddr.sa_data)[1],
                (int)((unsigned char *)&ifreq.ifr_hwaddr.sa_data)[2],
                (int)((unsigned char *)&ifreq.ifr_hwaddr.sa_data)[3],
                (int)((unsigned char *)&ifreq.ifr_hwaddr.sa_data)[4],
                (int)((unsigned char *)&ifreq.ifr_hwaddr.sa_data)[5]);
        }
    }

    return 0;
}

int set_nonblocking(int fd) {
    int val = fcntl(fd, F_GETFL);
    val |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, val) < 0) {
        LOG_SYS_ERR("set non block failed! fd: %d.", fd);
        return -1;
    }
    return 0;
}

int proc_client(const char *ip, int port, char *data) {
    LOG("run client...");

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
        LOG_SYS_ERR("create new socket failed!");
        return 0;
    }

    LOG("client connect to server! ip: %s, port: %d.", ip, port);

    ret = connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr));
    if (ret == -1) {
        LOG_SYS_ERR("connect failed!");
        close(fd);
        return 0;
    }

    LOG("begin to send data to server, fd: %d", fd);
    ret = write(fd, buf, strlen(buf));
    if (ret == -1) {
        LOG_SYS_ERR("send failed! fd: %d", fd);
        return 0;
    }
    LOG("send data to server, fd: %d, data: %s", fd, buf);

    ret = read(fd, buf, sizeof(buf));
    if (ret == -1) {
        LOG_SYS_ERR("read data failed! fd: %d.", fd);
        return 0;
    }
    LOG("read data from server, data: %s", buf);

    close(fd);
    LOG("close client, fd: %d", fd);
    return 1;
}