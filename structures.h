#ifndef STRUCTURES_H
#define STRUCTURES_H

#include "libraries.h"

/* Mutexes that control the program flow */
pthread_mutex_t fd_info_mutex;
pthread_mutex_t threads_mutex;
pthread_mutex_t log_mutex;
/* ------------------------------------- */

/* Type definition of the main structure of descriptors */
typedef struct fd_info_tag {
    int max_fd;
    int number_of_servers;
    int server_fds[FD_SETSIZE];
    int fds_mapping[FD_SETSIZE];
} fd_info_t;
/* ---------------------------------------------------- */

/* A global variable that counts valid requests */
int log_count;
/* -------------------------------------------- */

/* Type definition of the structure containing socket information */
typedef struct address_tag {
    int socket_fd;
    struct sockaddr_storage socket_address;
    socklen_t socket_size;
} address_t;
/* -------------------------------------------------------------- */

/* Type definitions of the arguments passed to threads */
typedef struct reply_thread_arg_tag {
    pthread_t *threads_ptr;
    int thread_index;

    int server_fd;
    int browser_fd;
} reply_thread_arg_t;

typedef struct request_thread_arg_tag {
    fd_info_t *fd_info_ptr;

    pthread_t *threads_ptr;
    int thread_index;

    address_t *browser_address_ptr;
} request_thread_arg_t;
/* --------------------------------------------------- */

/* Type definitions of structures required for some functions */
typedef struct buffer_tag {
    void *content;
    size_t length;
} buffer_t;

typedef struct request_tag {
    bool valid;
    char *host;
    char *port;
    char *path;
} request_t;
/* ---------------------------------------------------------- */

#endif
