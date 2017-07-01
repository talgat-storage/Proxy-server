#include "libraries.h"
#include "structures.h"
#include "functions.h"

void reap_children ( int );

int setup_server ( int argc, char *argv[] )
{
    int listen_fd;

    char *port;

    struct addrinfo hints, *servinfo, *p;

    int return_value;
    int yes;

    struct sigaction sig_action;

    if ( argc > 1 ) {
        port = (char *) malloc ( strlen ( argv[1] ) + 1 );

        strcpy ( port, argv[1] );
    }
    else {
        port = (char *) malloc ( strlen ( DEFAULT_PROXY_PORT ) + 1 );

        strcpy ( port, DEFAULT_PROXY_PORT );
    }

    memset ( (void *) &hints, 0, sizeof (struct addrinfo) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    return_value = getaddrinfo ( NULL, port, &hints, &servinfo );

    free ( (void *) port );

    if ( return_value != 0 ) {
        perror ( "getaddrinfo () error" );
        exit ( EXIT_FAILURE );
    }

    for ( p = servinfo; p != NULL; p = p->ai_next )
    {
        listen_fd = socket ( p->ai_family, p->ai_socktype, p->ai_protocol );

        if ( listen_fd == -1 )
            continue;

        yes = 1;

        if ( setsockopt ( listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int) ) == -1 ) {
            perror ( "setsockopt () error" );
            exit ( EXIT_FAILURE );
        }

        if ( bind ( listen_fd, p->ai_addr, p->ai_addrlen ) == -1 ) {
            close ( listen_fd );
            continue;
        }

        break;
    }

    freeaddrinfo ( servinfo );

    if ( p == NULL ) {
        fprintf ( stderr, "Proxy socket failed to bind\n" );
        exit ( EXIT_FAILURE );
    }

    if ( listen ( listen_fd, BACKLOG ) == -1 ) {
        perror ( "listen () error" );
        exit ( EXIT_FAILURE );
    }

    sig_action.sa_handler = reap_children;
    sigemptyset ( &sig_action.sa_mask );
    sig_action.sa_flags = SA_RESTART;

    if ( sigaction ( SIGCHLD, &sig_action, NULL ) == -1 ) {
        perror ( "sigaction () error" );
        exit ( EXIT_FAILURE );
    }

    return listen_fd;
}

address_t *setup_client ( request_t *request_ptr )
{
    address_t *server_address_ptr;

    struct addrinfo hints, *servinfo, *p;

    int return_value;

    server_address_ptr = allocate_address ();

    memset ( (void *) &hints, 0, sizeof (struct addrinfo) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    return_value = getaddrinfo ( request_ptr->host, request_ptr->port, &hints, &servinfo );

    if ( return_value != 0 ) {
        fprintf ( stderr, "getaddrinfo () error: %s\n", gai_strerror ( return_value ) );
        exit ( EXIT_FAILURE );
    }

    for ( p = servinfo; p != NULL; p = p->ai_next )
    {
        server_address_ptr->socket_fd = socket ( p->ai_family, p->ai_socktype, p->ai_protocol );

        if ( server_address_ptr->socket_fd == -1 ) {
            continue;
        }

        if ( connect ( server_address_ptr->socket_fd, p->ai_addr, p->ai_addrlen ) == -1 )
        {
            close ( server_address_ptr->socket_fd );
            continue;
        }

        break;
    }

    if ( p == NULL ) {
        fprintf ( stderr, "proxy failed to connect to server\n" );
        exit ( EXIT_FAILURE );
    }

    server_address_ptr->socket_address = *( (struct sockaddr_storage *)( p->ai_addr ) );

    server_address_ptr->socket_size = p->ai_addrlen;

    freeaddrinfo ( servinfo );

    return server_address_ptr;
}

void reap_children ( int signal )
{
    int saved_errno;

    saved_errno = errno;

    while ( waitpid ( -1, NULL, WNOHANG ) > 0 );

    errno = saved_errno;

}
