#ifndef __SERVER_H__
#define __SERVER_H__

#include "common.h"

#define BACKLOG 501          /* listen backlog. */
#define BUFFER_LEN 16 * 1024 /* send or recv buffer len. */
#define MAX_CLIENT_CNT 1024  /* max client's count. */
#define EVENTS_SIZE 1024     /* event's array size. */

int init_server(const char *ip, int port);
void run_server(int worker_index, int listen_fd);

#endif  //__SERVER_H__