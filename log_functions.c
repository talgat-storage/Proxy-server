#include "libraries.h"
#include "structures.h"
#include "functions.h"

void *get_in_addr ( struct sockaddr * );
in_port_t get_in_port ( struct sockaddr * );

void log_connection ( address_t *browser_address_ptr, address_t *server_address_ptr, request_t *request_ptr )
{
    FILE *log_file;

    char browser[INET6_ADDRSTRLEN];
    char server[INET6_ADDRSTRLEN];

    uint16_t browser_port;
    uint16_t server_port;

    /* Make the address information presentable */
    inet_ntop ( browser_address_ptr->socket_address.ss_family,
                get_in_addr ( (struct sockaddr *)&(browser_address_ptr->socket_address) ),
                browser, INET6_ADDRSTRLEN );

    inet_ntop ( server_address_ptr->socket_address.ss_family,
                get_in_addr ( (struct sockaddr *)&(server_address_ptr->socket_address) ),
                server, INET6_ADDRSTRLEN );

    browser_port = ntohs ( get_in_port ( (struct sockaddr *)&(browser_address_ptr->socket_address) ) );

    server_port = ntohs ( get_in_port ( (struct sockaddr *)&(server_address_ptr->socket_address) ) );
    /* ---------------------------------------- */

    /* Critical section of log file */
    pthread_mutex_lock ( &log_mutex );

        /* Get the access to the shared log file */
        log_file = fopen ( LOG_FILENAME, "a" );
        if ( log_file == NULL ) {
            perror ( "fopen () error" );
            exit ( EXIT_FAILURE );
        }
        /* ------------------------------------- */

        /* Increment the value of the global variable */
        log_count += 1;
        /* ------------------------------------------ */

        /* Write the information into the log file */
        fprintf ( log_file, "%d %s:%hu %s:%hu GET %s\n", log_count,
                                                         browser, browser_port,
                                                         server, server_port,
                                                         request_ptr->path );
        /* --------------------------------------- */

        /* Close the log file */
        fclose ( log_file );
        /* ------------------ */

    pthread_mutex_unlock ( &log_mutex );
    /* ---------------------------- */

    return;
}

/* Get sockaddr, IPv4 or IPv6 */
void *get_in_addr ( struct sockaddr *sa )
{
    if ( sa->sa_family == AF_INET ) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

in_port_t get_in_port ( struct sockaddr *sa )
{
    if ( sa->sa_family == AF_INET ) {
        return ((struct sockaddr_in *)sa)->sin_port;
    }
    return ((struct sockaddr_in6 *)sa)->sin6_port;
}
