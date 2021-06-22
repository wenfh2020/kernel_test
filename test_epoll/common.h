#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

enum LOG_TYPE {
    LT_CLIENT = 0,
    LT_SERVER = 1
};

#define LOG(args...) log_data(LT_SERVER, 0, __LINE__, __FUNCTION__, ##args)
#define LOG_SYS_ERR(args...) log_data(LT_SERVER, 1, __LINE__, __FUNCTION__, ##args)

#define LOGC(args...) log_data(LT_CLIENT, 0, __LINE__, __FUNCTION__, ##args)
#define LOGC_SYS_ERR(args...) log_data(LT_CLIENT, 1, __LINE__, __FUNCTION__, ##args)

int log_data(int is_server, int is_err,
             int file_line, const char *func, const char *fmt, ...);

int bring_up_net_interface(const char *ip);

#endif  //__COMMON_H__