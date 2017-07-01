#include "libraries.h"
#include "structures.h"
#include "functions.h"

fd_set *capture_descriptors ( fd_info_t *fd_info_ptr, int listen_fd )
{
    fd_set *read_fds_ptr;

    int i;

    int number_of_set_servers;

    read_fds_ptr = (fd_set *) malloc ( sizeof (fd_set) );

    FD_ZERO ( read_fds_ptr );

    FD_SET ( listen_fd, read_fds_ptr );

    number_of_set_servers = 0;

    /* Critical section of descriptors info */
    pthread_mutex_lock ( &fd_info_mutex );

        for ( i = 0; i < FD_SETSIZE; i++ )
        {
            if ( number_of_set_servers == fd_info_ptr->number_of_servers )
                break;

            if ( fd_info_ptr->server_fds[i] != 0 )
            {
                FD_SET ( fd_info_ptr->server_fds[i], read_fds_ptr );

                number_of_set_servers += 1;
            }
        }

    pthread_mutex_unlock ( &fd_info_mutex );
    /* ------------------------------------ */

    return read_fds_ptr;
}

int find_ready_server_fd ( fd_set *read_fds_ptr, fd_info_t *fd_info_ptr )
{
    int server_fd;
    int i;

    /* Critical section of descriptors info */
    pthread_mutex_lock ( &fd_info_mutex );

        if ( fd_info_ptr->number_of_servers != 0 )
        {
            for ( i = 0; i < FD_SETSIZE; i++ )
            {
                if ( fd_info_ptr->server_fds[i] == 0 )
                    continue;

                if ( FD_ISSET ( fd_info_ptr->server_fds[i], read_fds_ptr ) )
                    break;
            }

            server_fd = fd_info_ptr->server_fds[i];
        }

        else
            server_fd = 0;

    pthread_mutex_unlock ( &fd_info_mutex);
    /* ------------------------------------ */

    return server_fd;
}

int find_mapping ( fd_info_t *fd_info_ptr, int server_fd )
{
    int browser_fd;

    /* Critical section of descriptors info */
    pthread_mutex_lock ( &fd_info_mutex );

        browser_fd = fd_info_ptr->fds_mapping[server_fd];

    pthread_mutex_unlock ( &fd_info_mutex );
    /* ------------------------------------ */

    return browser_fd;
}

void delete_fd_info ( fd_info_t *fd_info_ptr, int server_fd, int listen_fd )
{
    int new_max_fd, checked_servers;
    int i;

    /* Critical section of descriptors info */
    pthread_mutex_lock ( &fd_info_mutex );

        if ( fd_info_ptr->max_fd == server_fd )
        {
            new_max_fd = 0;
            checked_servers = 0;

            for ( i = 0; i < FD_SETSIZE; i++ )
            {
                if ( checked_servers == fd_info_ptr->number_of_servers )
                    break;

                if ( fd_info_ptr->server_fds[i] == 0 )
                    continue;

                checked_servers += 1;

                if ( fd_info_ptr->server_fds[i] > new_max_fd && fd_info_ptr->server_fds[i] != server_fd )
                {
                    new_max_fd = fd_info_ptr->server_fds[i];
                }
            }

            if ( listen_fd > new_max_fd )
                new_max_fd = listen_fd;

            fd_info_ptr->max_fd = new_max_fd;
        }

        for ( i = 0; i < FD_SETSIZE; i++ )
        {
            if ( fd_info_ptr->server_fds[i] == server_fd )
                break;
        }

        fd_info_ptr->server_fds[i] = 0;

        fd_info_ptr->number_of_servers -= 1;

        fd_info_ptr->fds_mapping[server_fd] = 0;

    pthread_mutex_unlock ( &fd_info_mutex );
    /* ------------------------------------ */

    return;
}

void add_server_descriptor ( fd_info_t *fd_info_ptr, int server_fd, int browser_fd )
{
    int i;

    /* Critical section of descriptors info */
    pthread_mutex_lock ( &fd_info_mutex );

        if ( server_fd > fd_info_ptr->max_fd )
            fd_info_ptr->max_fd = server_fd;

        for ( i = 0; i < FD_SETSIZE; i++ )
        {
            if ( fd_info_ptr->server_fds[i] == 0 )
                break;
        }

        fd_info_ptr->server_fds[i] = server_fd;

        fd_info_ptr->number_of_servers += 1;

        fd_info_ptr->fds_mapping[server_fd] = browser_fd;

    pthread_mutex_unlock ( &fd_info_mutex );
    /* ------------------------------------ */

    return;
}
