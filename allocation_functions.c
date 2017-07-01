#include "libraries.h"
#include "structures.h"
#include "functions.h"

reply_thread_arg_t *allocate_reply_thread_arg ( pthread_t *threads_ptr, int thread_index, int server_fd, int browser_fd )
{
    reply_thread_arg_t *reply_thread_arg_ptr;

    reply_thread_arg_ptr = (reply_thread_arg_t *) malloc ( sizeof (reply_thread_arg_t) );

    reply_thread_arg_ptr->threads_ptr = threads_ptr;

    reply_thread_arg_ptr->thread_index = thread_index;

    reply_thread_arg_ptr->server_fd = server_fd;

    reply_thread_arg_ptr->browser_fd = browser_fd;

    return reply_thread_arg_ptr;
}

void free_reply_thread_arg ( reply_thread_arg_t *reply_thread_arg_ptr )
{
    free ( (void *) reply_thread_arg_ptr );

    return;
}

address_t *allocate_address ()
{
    address_t *address_ptr;

    address_ptr = (address_t *) malloc ( sizeof (address_t) );

    memset ( (void *) address_ptr, 0, sizeof (address_t) );

    address_ptr->socket_size = sizeof ( struct sockaddr_storage );

    return address_ptr;
}

request_thread_arg_t *allocate_request_thread_arg ( fd_info_t *fd_info_ptr, pthread_t *threads_ptr, int thread_index, address_t *browser_address_ptr )
{
    request_thread_arg_t *request_thread_arg_ptr;

    request_thread_arg_ptr = (request_thread_arg_t *) malloc ( sizeof (request_thread_arg_t) );

    request_thread_arg_ptr->fd_info_ptr = fd_info_ptr;

    request_thread_arg_ptr->threads_ptr = threads_ptr;

    request_thread_arg_ptr->thread_index = thread_index;

    request_thread_arg_ptr->browser_address_ptr = browser_address_ptr;

    return request_thread_arg_ptr;
}

void free_request_thread_arg ( request_thread_arg_t *request_thread_arg_ptr )
{
    free ( (void *) request_thread_arg_ptr );

    return;
}

void free_address ( address_t *address_ptr )
{
    free ( (void *) address_ptr );

    return;
}
