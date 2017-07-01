#include "libraries.h"
#include "structures.h"
#include "functions.h"

int find_header_length ( buffer_t * );
int find_content_length ( buffer_t *, int );

void receive_buffer ( int socket_fd, buffer_t * buffer_ptr )
{
    int socket_flags;
    ssize_t bytes_read;

    int header_length;
    int content_length;

    /* Make the socket non-blocking */
    socket_flags = fcntl ( socket_fd, F_GETFL, 0 );

    if ( fcntl ( socket_fd, F_SETFL, socket_flags | O_NONBLOCK ) ) {
        perror ( "fcntl () error" );
        exit ( EXIT_FAILURE );
    }
    /* ---------------------------- */

    /* Initialize the buffer length and prepare the buffer space */
    buffer_ptr->length = 0;
    buffer_ptr->content = malloc ( PACKET_SIZE );
    /* --------------------------------------------------------- */

    /* Dummy assignments */
    header_length = 0;
    content_length = 0;
    /* ----------------- */

    while ( true )
    {
        bytes_read = read ( socket_fd, buffer_ptr->content + buffer_ptr->length, PACKET_SIZE );

        /* If connection is closed, adjust the buffer to its actual size */
        if ( bytes_read == 0 )
        {
            buffer_ptr->content = realloc ( buffer_ptr->content, buffer_ptr->length );

            break;
        }
        /* ------------------------------------------------------------- */

        /* If nothing was ever received yet, check again */
        else if ( bytes_read == -1 && errno == EAGAIN && buffer_ptr->length == 0 )
        {
            continue;
        }
        /* --------------------------------------------- */

        /* If nothing is received, check if it is the end of the stream */
        else if ( bytes_read == -1 && errno == EAGAIN )
        {
            /* Adjust the buffer to its actual size */
            buffer_ptr->content = realloc ( buffer_ptr->content, buffer_ptr->length );
            /* ------------------------------------ */

            header_length = find_header_length ( buffer_ptr );

            /* If CRLF + CRLF was not found, wait further */
            if ( header_length == 0 )
            {
                /* Prepare the buffer for a new packet */
                buffer_ptr->content = realloc ( buffer_ptr->content, buffer_ptr->length + PACKET_SIZE );
                /* ----------------------------------- */
                continue;
            }
            /* ------------------------------------------ */

            /* Check if there is any notion about content length */
            content_length = find_content_length ( buffer_ptr, header_length );
            /* ------------------------------------------------- */

            /* VERY RISKY BUT SIMPLE (MAY LEAD TO BUGS) */
            if ( content_length + header_length == buffer_ptr->length )
                break;
            /* ---------------------------------------- */

            /* Prepare the buffer for a new packet */
            buffer_ptr->content = realloc ( buffer_ptr->content, buffer_ptr->length + PACKET_SIZE );
            /* ----------------------------------- */
            continue;
        }

        /* If something was received, adjust the buffer length and prepare the buffer for new coming */
        else if ( bytes_read > 0 )
        {
            /* Update the buffer length */
            buffer_ptr->length += bytes_read;
            /* ------------------------ */

            /* Prepare the buffer for a new packet */
            buffer_ptr->content = realloc ( buffer_ptr->content, buffer_ptr->length + PACKET_SIZE );
            /* ----------------------------------- */
        }
        /* ----------------------------------------------------------------------------------------- */

        else
        {
            perror ( "read () error" );
            exit ( EXIT_FAILURE );
        }
    }

    return;
}

int find_header_length ( buffer_t *buffer_ptr )
{
    int header_length;

    char *end;

    /* In order to avoid any complaints from strstr () function */
    buffer_ptr->content = realloc ( buffer_ptr->content, buffer_ptr->length + 1 );

    ( (char *)buffer_ptr->content )[buffer_ptr->length] = '\0';
    /* -------------------------------------------------------- */

    /* If CRLF + CRLF is found, check if the string belongs to the buffer and calculate its length */
    if ( strstr ( (char *) buffer_ptr->content, "\r\n\r\n" ) != NULL )
    {
        end = strstr ( (char *) buffer_ptr->content, "\r\n\r\n" ) + strlen ( "\r\n\r\n" );

        header_length = (int) ( end - (char *) buffer_ptr->content );

        if ( header_length > buffer_ptr->length ) /* Sometimes it can find the string outside the buffer */
            header_length = 0;
    }
    else
    {
        header_length = 0;
    }

    buffer_ptr->content = realloc ( buffer_ptr->content, buffer_ptr->length );

    return header_length;
}

int find_content_length ( buffer_t *buffer_ptr, int header_length )
{
    int content_length;

    char *beginning;
    char *end;

    char *number_string;

    /* In order to avoid any complaints from strstr () function */
    buffer_ptr->content = realloc ( buffer_ptr->content, buffer_ptr->length + 1 );

    ( (char *)buffer_ptr->content )[buffer_ptr->length] = '\0';
    /* -------------------------------------------------------- */

    /* If Content-Length string is found, check if it belongs to the buffer and parse its value */
    if ( strstr ( (char *) buffer_ptr->content, "Content-Length: ") != NULL )
    {
        beginning = strstr ( (char *) buffer_ptr->content, "Content-Length: " ) + strlen ( "Content-Length: " );

        if ( (void *) beginning > buffer_ptr->content + header_length ) /* Sometimes it can find the string outside the buffer */
            content_length = 0;

        else
        {
            end = strstr ( beginning, "\r\n" ) - 1;

            number_string = (char *) malloc ( (size_t) (end - beginning) + 1 + 1 );

            memmove ( (void *) number_string, (void *) beginning, (size_t) (end - beginning) + 1 );

            number_string[ (size_t) (end - beginning) + 1 ] = '\0';

            content_length = atoi ( number_string );

            free ( (void *) number_string );
        }
    }

    else
    {
        content_length = 0;
    }

    buffer_ptr->content = realloc ( buffer_ptr->content, buffer_ptr->length );

    return content_length;
}

void send_buffer ( int socket_fd, buffer_t *buffer_ptr )
{
    int socket_flags;

    ssize_t bytes_sent;
    ssize_t just_sent;
    ssize_t bytes_left;

    int header_length;

    /* Reset the socket to blocking */
    socket_flags = fcntl ( socket_fd, F_GETFL, 0 );

    if ( fcntl ( socket_fd, F_SETFL, socket_flags & (~O_NONBLOCK) ) ) {
        perror ( "fcntl () error" );
        exit ( EXIT_FAILURE );
    }
    /* ---------------------------- */

    bytes_sent = 0;

    bytes_left = (ssize_t) buffer_ptr->length;

    while ( bytes_left != 0 )
    {
        just_sent = write ( socket_fd, buffer_ptr->content + (size_t) bytes_sent, (size_t) bytes_left );

        if ( just_sent == -1 ) {
            perror ( "write () error" );
            exit ( EXIT_FAILURE );
        }

        bytes_sent += just_sent;

        bytes_left -= just_sent;
    }

    header_length = find_header_length ( buffer_ptr );

    return;
}

void free_buffer ( buffer_t *buffer_ptr )
{
    free ( buffer_ptr->content );

    return;
}
