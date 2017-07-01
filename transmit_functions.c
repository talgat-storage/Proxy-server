#include "libraries.h"
#include "structures.h"
#include "functions.h"

void *transmit_reply ( void *argument )
{
    reply_thread_arg_t *reply_thread_arg_ptr;

    int browser_fd;
    int server_fd;

    buffer_t buffer;

    /* Cast the passed argument */
    reply_thread_arg_ptr = (reply_thread_arg_t *) argument;
    /* ------------------------ */

    /* Detach the thread */
    pthread_detach ( pthread_self () );
    /* ----------------- */

    /* Push the cleanup function of the thread */
    pthread_cleanup_push ( cleanup_reply_thread, (void *) reply_thread_arg_ptr );
    /* --------------------------------------- */

    /* Get descriptors of the server and the browser */
    server_fd = reply_thread_arg_ptr->server_fd;

    browser_fd = reply_thread_arg_ptr->browser_fd;
    /* --------------------------------------------- */

    /* Receive the buffer from the server */
    receive_buffer ( server_fd, &buffer );

    if ( buffer.length == 0 )
    {
        close ( server_fd );
        close ( browser_fd );
        free_buffer ( &buffer );
        pthread_exit ( NULL );
        return NULL;
    }
    /* ---------------------------------- */

    /* Send the reply to the browser */
    send_buffer ( browser_fd, &buffer );
    /* ----------------------------- */

    close ( server_fd );
    close ( browser_fd );
    free_buffer ( &buffer );

    pthread_cleanup_pop ( 1 );

    return NULL;
}

void *transmit_request ( void *argument )
{
    request_thread_arg_t *request_thread_arg_ptr;

    fd_info_t *fd_info_ptr;

    address_t *browser_address_ptr;

    buffer_t buffer;

    request_t request;

    address_t *server_address_ptr;

    /* Cast the passed argument */
    request_thread_arg_ptr = (request_thread_arg_t *) argument;
    /* ------------------------ */

    /* Detach the thread */
    pthread_detach ( pthread_self () );
    /* -------------------------- */

    /* Push the cleanup function of the thread */
    pthread_cleanup_push ( cleanup_request_thread, (void *) request_thread_arg_ptr );
    /* --------------------------------------- */

    /* Get the pointers to the descriptors structure and to the browser address */
    fd_info_ptr = request_thread_arg_ptr->fd_info_ptr;

    browser_address_ptr = request_thread_arg_ptr->browser_address_ptr;
    /* ------------------------------------------------------------------------ */

    /* Receive the buffer from the browser */
    receive_buffer ( browser_address_ptr->socket_fd, &buffer );

    if ( buffer.length == 0 )
    {
        close ( browser_address_ptr->socket_fd );
        free_address ( browser_address_ptr );
        free_buffer ( &buffer );
        pthread_exit ( NULL );
        return NULL;
    }
    /* ----------------------------------- */

    /* Parse the request */
    parse_request ( &buffer, &request );

    if ( request.valid == false ) {
        close ( browser_address_ptr->socket_fd );
        free_address ( browser_address_ptr );
        free_buffer ( &buffer );
        free_request ( &request );
        pthread_exit ( NULL );
        return NULL;
    }
    /* ----------------- */

    /* Connect the proxy to the server */
    server_address_ptr = setup_client ( &request );
    /* ------------------------------- */

    /* Add the descriptor */
    add_server_descriptor ( fd_info_ptr,
                            server_address_ptr->socket_fd,
                            browser_address_ptr->socket_fd ); /* safe to invoke (no race condition) */
    /* ------------------ */

    /* Edit and send the request to the server */
    edit_request ( &buffer, &request );

    send_buffer ( server_address_ptr->socket_fd, &buffer );
    /* --------------------------------------- */

    /* Write the connection information into the log file */
    log_connection ( browser_address_ptr,
                     server_address_ptr,
                     &request );
    /* -------------------------------------------------- */

    free_address ( browser_address_ptr );
    free_address ( server_address_ptr );
    free_buffer ( &buffer );
    free_request ( &request );

    pthread_cleanup_pop ( 1 );

    return NULL;
}
