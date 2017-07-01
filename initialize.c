#include "libraries.h"
#include "structures.h"
#include "functions.h"

void initialize_proxy ( fd_info_t *fd_info_ptr, pthread_t *threads_ptr )
{
    FILE *log_file;

    log_file = fopen ( LOG_FILENAME, "w" );

    fclose ( log_file );

    log_count = 0;

    memset ( (void *) fd_info_ptr, 0, sizeof (fd_info_t) );

    memset ( (void *) threads_ptr, 0, FD_SETSIZE );

    pthread_mutex_init ( &fd_info_mutex, NULL );

    pthread_mutex_init ( &threads_mutex, NULL );

    pthread_mutex_init ( &log_mutex, NULL );

    return;
}
