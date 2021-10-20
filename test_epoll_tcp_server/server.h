#ifndef __SERVER_H__
#define __SERVER_H__

#include "common.h"

#define BACKLOG 501          /* listen backlog. */
#define BUFFER_LEN 16 * 1024 /* send or recv buffer len. */
#define MAX_CLIENT_CNT 1024  /* max client's count. */
#define EVENTS_SIZE 1024     /* event's array size. */

/* client's info. */
typedef struct client_s {
    int fd;                /* socket's file descriptor. */
    char rbuf[BUFFER_LEN]; /* read buffer. */
    int rlen;              /* read buffer len. */
    char wbuf[BUFFER_LEN]; /* write buffer. */
    int wlen;              /* write buffer len. */
    int events;            /* control EPOLLIN/EPOLLOUT events. */
} client_t;

client_t *g_clients[MAX_CLIENT_CNT];      /* client's array. */
client_t *add_client(int fd, int events); /* add a new client. */
client_t *get_client(int fd);             /* get client ptr. */
int del_client(int fd);                   /* close fd and clear data. */

int init_epoll(int listen_fd);
int init_server(int worker_index, const char *ip, int port);
void run_server(int worker_index);

int read_data(int fd);          /* read data into buffer. */
int write_data(int fd);         /* write data to client. */
int accept_data(int listen_fd); /* accept a new client. */
int handle_data(int fd);        /* hanlde user's logic. */

int handle_write_events(int fd);

#endif  //__SERVER_H__