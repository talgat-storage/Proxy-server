#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "libraries.h"
#include "structures.h"

#define LOG_FILENAME "proxy.log"
#define DEFAULT_PROXY_PORT "1234"
#define BACKLOG (128)
#define PACKET_SIZE (5242880)

void initialize_proxy ( fd_info_t *, pthread_t * );

int setup_server ( int, char ** );
address_t *setup_client ( request_t * );

fd_set *capture_descriptors ( fd_info_t *, int );
int find_ready_server_fd ( fd_set *, fd_info_t * );
int find_mapping ( fd_info_t *, int );
void delete_fd_info ( fd_info_t *, int, int );
void add_server_descriptor ( fd_info_t *, int, int );

reply_thread_arg_t *allocate_reply_thread_arg ( pthread_t *, int, int, int );
address_t *allocate_address ();
request_thread_arg_t *allocate_request_thread_arg ( fd_info_t *, pthread_t *, int, address_t * );
void free_reply_thread_arg ( reply_thread_arg_t * );
void free_address ( address_t * );
void free_request_thread_arg ( request_thread_arg_t * );

int find_free_thread_index ( pthread_t * );
void cleanup_reply_thread ( void * );
void cleanup_request_thread ( void * );

void *transmit_request ( void * );
void *transmit_reply ( void * );

void receive_buffer ( int, buffer_t * );
void send_buffer ( int, buffer_t * );
void free_buffer ( buffer_t * );

void parse_request ( buffer_t *, request_t * );
void free_request ( request_t * );
void edit_request ( buffer_t *, request_t * );

void log_connection ( address_t *, address_t *, request_t * );

#endif
