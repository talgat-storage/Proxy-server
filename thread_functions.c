#include "libraries.h"
#include "structures.h"
#include "functions.h"

int find_free_thread_index ( pthread_t *threads_ptr )
{
    int thread_index;

    /* Critical section of threads info */
    pthread_mutex_lock ( &threads_mutex );

        for ( thread_index = 0; thread_index < FD_SETSIZE; thread_index++ )
        {
            if ( threads_ptr[thread_index] == 0 )
                break;
        }

    pthread_mutex_unlock ( &threads_mutex );
    /* -------------------------------- */

    return thread_index;
}

void cleanup_reply_thread ( void *argument )
{
    reply_thread_arg_t *reply_thread_arg_ptr;
    pthread_t *threads_ptr;
    int thread_index;

    reply_thread_arg_ptr = (reply_thread_arg_t *) argument;
    threads_ptr = reply_thread_arg_ptr->threads_ptr;
    thread_index = reply_thread_arg_ptr->thread_index;

    /* Critical section of threads info */
    pthread_mutex_lock ( &threads_mutex );

        threads_ptr[thread_index] = 0;

    pthread_mutex_unlock ( &threads_mutex );
    /* -------------------------------- */

    free_reply_thread_arg ( reply_thread_arg_ptr );

    return;
}

void cleanup_request_thread ( void *argument )
{
    request_thread_arg_t *request_thread_arg_ptr;
    pthread_t *threads_ptr;
    int thread_index;

    request_thread_arg_ptr = (request_thread_arg_t *) argument;
    threads_ptr = request_thread_arg_ptr->threads_ptr;
    thread_index = request_thread_arg_ptr->thread_index;

    /* Critical section of threads info */
    pthread_mutex_lock ( &threads_mutex );

        threads_ptr[thread_index] = 0;

    pthread_mutex_unlock ( &threads_mutex );
    /* -------------------------------- */

    free_request_thread_arg ( request_thread_arg_ptr );

    return;
}

