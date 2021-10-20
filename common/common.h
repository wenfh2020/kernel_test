#ifndef __COMMON_H__
#define __COMMON_H__

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LOG(args...) log_data(0, __FILE__, __FUNCTION__, __LINE__, 0, ##args)
#define LOG_SYS_ERR(args...) log_data(1, __FILE__, __FUNCTION__, __LINE__, errno, ##args)

int log_data(int is_err, const char *file,
             const char *func, int file_line, int err, const char *fmt, ...);

int bring_up_net_interface(const char *ip);

int set_nonblocking(int fd); /* set fd unblock. */

/* client send 'hello' to server. */
int proc_client(const char *ip, int port, char *data);

#endif  //__COMMON_H__