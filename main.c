#include "libraries.h"
#include "structures.h"
#include "functions.h"

int main ( int argc, char *argv[] )
{
    /* Main structures that control multi-threading */
    fd_info_t fd_info;

    int thread_index;
    pthread_t threads_ptr[FD_SETSIZE];
    /* -------------------------------------------- */

    /* For setting up the proxy as a listening server */
    int listen_fd;
    /* ---------------------------------------------- */

    /* For using select () function */
    fd_set *read_fds_ptr;
    struct timeval timer;
    int select_value;
    /* ---------------------------- */

    /* For connections from servers */
    int server_fd;
    int browser_fd;

    reply_thread_arg_t *reply_thread_arg_ptr;
    /* ---------------------------- */

    /* For connections from browsers */
    address_t *browser_address_ptr;

    request_thread_arg_t *request_thread_arg_ptr;
    /* ----------------------------- */

    /* Initialize structures, variables and mutexes */
    initialize_proxy ( &fd_info, threads_ptr );
    /* -------------------------------------------- */

    /* Make the proxy a listening server */
    listen_fd = setup_server ( argc, argv );
    /* --------------------------------- */

    /* Modify the max_fd to listen_fd */
    fd_info.max_fd = listen_fd;
    /* ------------------------------ */

    while ( true )
    {
        /* Capture descriptors of servers and the listening socket */
        read_fds_ptr = capture_descriptors ( &fd_info, listen_fd ); /* safe to invoke (no race condition) */
        /* ------------------------------------------------------- */

        /* Initialize the timer for the select () function */
        timer.tv_sec = 0;
        timer.tv_usec = 0;
        /* ----------------------------------------------- */

        /* Check if there are any ready descriptors */
        select_value = select ( fd_info.max_fd + 1, read_fds_ptr, NULL, NULL, &timer );
        /* ---------------------------------------- */

        if ( select_value == -1 ) {
            perror ( "select () error" );
            exit ( EXIT_FAILURE );
        }

        /* If there are no ready descriptors, check again */
        if ( select_value == 0 )
        {
            free ( (void *) read_fds_ptr );
            continue;
        }
        /* ---------------------------------------------- */

        /* When the program reaches this point, there should be at least one ready descriptor */
        assert ( select_value > 0 );
        /* ---------------------------------------------------------------------------------- */

        /* There are three possible options:
        1. A new request from some browser
        2. A new reply from some server
        3. Error */

        /* Check if there is any server sending its reply. If no, 0 is returned */
        server_fd = find_ready_server_fd ( read_fds_ptr, &fd_info ); /* safe to invoke (no race condition) */
        /* -------------------------------------------------------------------- */

        if ( server_fd != 0 )
        {
            free ( (void *) read_fds_ptr );

            /* Find the corresponding browser that should receive the reply */
            browser_fd = find_mapping ( &fd_info, server_fd ); /* safe to invoke (no race condition) */
            /* ------------------------------------------------------------ */

            /* Delete all information about the server from the main descriptors structure
            in order to avoid two threads transmitting the reply from the server to the browser */
            delete_fd_info ( &fd_info, server_fd, listen_fd ); /* safe to invoke (no race condition) */
            /* -------------------------------------------------------------------------------- */

            /* Find one available thread */
            thread_index = find_free_thread_index ( threads_ptr );
            /* ------------------------- */

            /* Prepare the reply thread argument */
            reply_thread_arg_ptr = allocate_reply_thread_arg ( threads_ptr, thread_index, server_fd, browser_fd );
            /* --------------------------------- */

            /* Create the new thread and pass the argument to it */
            pthread_create ( &(threads_ptr[thread_index]), NULL, &(transmit_reply), (void *) reply_thread_arg_ptr );
            /* ------------------------------------------------- */
        }

        else if ( FD_ISSET ( listen_fd, read_fds_ptr ) )
        {
            free ( (void *) read_fds_ptr );

            /* Prepare the structure to receive information about the browser (needed for log) */
            browser_address_ptr = allocate_address ();
            /* ------------------------------------------------------------------------------- */

            /* Accept a new connection from the browser */
            browser_address_ptr->socket_fd = accept ( listen_fd,
                                                      (struct sockaddr *) &(browser_address_ptr->socket_address),
                                                      &(browser_address_ptr->socket_size) );
            /* ---------------------------------------- */

            if ( browser_address_ptr->socket_fd == -1 ) {
                perror ( "accept () error" );
                exit ( EXIT_FAILURE );
            }

            /* Find one available thread */
            thread_index = find_free_thread_index ( threads_ptr );
            /* ------------------------- */

            /* Prepare the request thread argument */
            request_thread_arg_ptr = allocate_request_thread_arg ( &fd_info, threads_ptr, thread_index, browser_address_ptr );
            /* ----------------------------------- */

            /* Create the new thread and pass the argument to it */
            pthread_create ( &(threads_ptr[thread_index]), NULL, &(transmit_request), (void *) request_thread_arg_ptr );
            /* ------------------------------------------------- */
        }
        else
        {
            /* Impossible to occur but still */
            fprintf ( stderr, "Error in the code\n" );
            exit ( EXIT_FAILURE );
            /* ----------------------------- */
        }
    }

    return 0;
}
