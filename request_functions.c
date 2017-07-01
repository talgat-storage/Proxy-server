#include "libraries.h"
#include "structures.h"
#include "functions.h"

void parse_request ( buffer_t *buffer, request_t *request )
{
    int count;
    char *message;
    size_t message_length;
    size_t i;

    char *url;
    size_t url_length;

    struct http_parsed_url *url_parsed;

    message = (char *)(buffer->content);
    message_length = buffer->length;

    request->valid = false;
    request->host = NULL;
    request->path = NULL;
    request->port = NULL;

    /* Eat tabs and whitespace */
    for ( i = 0; i < message_length; i++ )
    {
        if ( message[i] == '\n' || message[i] == '\r' ) {
            request->valid = false;
            return;
        }
        if ( message[i] != '\t' && message[i] != ' ' )
            break;
    }

    message = message + i;
    message_length -= i;
    /* ----------------------- */

    /* Check if there is GET */
    if ( message_length < 3
        || strncmp ( message, "GET", 3 ) != 0 ) {
            request->valid = false;
            return;
    }

    message = message + 3;
    message_length -= 3;
    /* --------------------- */

    /* Eat tabs and whitespace */
    for ( i = 0; i < message_length; i++ )
    {
        if ( message[i] == '\n' || message[i] == '\r' ) {
            request->valid = false;
            return;
        }
        if ( message[i] != '\t' && message[i] != ' ' )
            break;
    }

    if ( i == 0 ) {
        request->valid = false;
        return;
    }
    message = message + i;
    message_length -= i;
    /* ----------------------- */

    /* Calculate the length of URL */
    for ( i = 0; i < message_length; i++ )
    {
        if ( message[i] == '\n' || message[i] == '\r' ) {
            request->valid = false;
            return;
        }
        if ( message[i] == '\t' || message[i] == ' ' )
            break;
    }

    if ( i == 0 ) {
        request->valid = false;
        return;
    }

    url_length = i;
    /* --------------------------- */

    /* Copy the URL from buffer into allocated space */
    url = (char *) malloc ( url_length + 1 );

    memmove ( url, message, url_length );

    url[url_length] = '\0';

    message = message + url_length;
    message_length -= url_length;
    /* --------------------------------------------- */

    /* Check if the scheme is http */
    url_parsed = parse_url ( url );

    if ( url_parsed == NULL || url_parsed->scheme == NULL ||
         strncmp ( url_parsed->scheme, "http", 4 ) != 0 ||
         url_parsed->host == NULL ) {
            request->valid = false;
            return;
         }

    free ( (void *) url );
    /* --------------------------- */

    /* Extract information about the host */
    request->host = (char *)malloc ( strlen ( url_parsed->host ) + 1 );
    strcpy ( request->host, url_parsed->host );
    /* ---------------------------------- */

    /* Extract information about the port */
    if ( url_parsed->port != NULL ) {
        request->port = (char *)malloc ( strlen ( url_parsed->port ) + 1 );
        strcpy ( request->port, url_parsed->port );
    }
    else {
        request->port = (char *)malloc ( strlen ("80") + 1 );
        strcpy ( request->port, "80" );
    }
    /* ---------------------------------- */

    /* Extract information about the path */
    if ( url_parsed->path != NULL ) {
        request->path = (char *)malloc ( 1 + strlen (url_parsed->path) + 1 );
        if ( sprintf ( request->path, "/%s", url_parsed->path ) < 0 ) {
                perror ( "sprintf () error" );
                exit ( EXIT_FAILURE );
            }
    }
    else {
        request->path = (char *)malloc ( 1 + 1 );
        if ( sprintf ( request->path, "/" ) < 0 ) {
                perror ( "sprintf () error" );
                exit ( EXIT_FAILURE );
            }
    }
    /* ---------------------------------- */

    /* Extract information about the query */
    if ( url_parsed->query != NULL ) {
        request->path = (char *)realloc ( request->path,
                                          strlen (request->path) +
                                          strlen (url_parsed->query) + 1 );
        strcat ( request->path, url_parsed->query );
    }
    /* ----------------------------------- */

    /* Extract information about the fragment */
    if ( url_parsed->fragment != NULL ) {
        request->path = (char *)realloc ( request->path,
                                          strlen (request->path) +
                                          strlen (url_parsed->fragment) + 1 );
        strcat ( request->path, url_parsed->fragment );
    }
    /* -------------------------------------- */

    /* Eat tabs and whitespace */
    for ( i = 0; i < message_length; i++ )
    {
        if ( message[i] == '\n' || message[i] == '\r' ) {
            request->valid = false;
            return;
        }
        if ( message[i] != '\t' && message[i] != ' ' )
            break;
    }

    if ( i == 0 ) {
        request->valid = false;
        return;
    }
    message = message + i;
    message_length -= i;
    /* ----------------------- */

    /* Check if there is HTTP/1.0 or HTTP/1.1 */
    if ( message_length < 8
        || ( strncmp ( message, "HTTP/1.0", 8 ) != 0 &&
             strncmp ( message, "HTTP/1.1", 8 ) != 0 ) ) {
                 request->valid = false;
                 return;
    }

    message = message + 8;
    message_length -= 8;
    /* --------------------- */

    /* Eat tabs and whitespace */
    for ( i = 0; i < message_length; i++ )
    {
        if ( message[i] != '\t' && message[i] != ' ' )
            break;
    }

    message = message + i;
    message_length -= i;
    /* ----------------------- */

    /* Check if there is \r\n */
    if ( message_length < 2
        || strncmp ( message, "\r\n", 2 ) != 0 ) {
            request->valid = false;
            return;
    }

    message = message + 2;
    message_length -= 2;
    /* ---------------------- */

    /* Check if there is a trailing \r\n */
    if ( message_length < 2 ) {
        request->valid = false;
        return;
    }
    if ( message[message_length - 2] != '\r' ||
         message[message_length - 1] != '\n' ) {
            request->valid = false;
            return;
         }
    /* --------------------------------- */

    http_parsed_url_free ( url_parsed );
    request->valid = true;
    return;
}

void free_request ( request_t *request  )
{
    if ( request->host != NULL ) {
        free ( request->host );
    }
    if ( request->port != NULL ) {
        free ( request->port );
    }
    if ( request->path != NULL ) {
        free ( request->path );
    }

    return;
}

void edit_request ( buffer_t * buffer, request_t *request )
{
    void *new_buffer;
    size_t new_buffer_length;

    char *cut_beginning;
    char *cut_end;

    cut_beginning = strstr ( (char *)(buffer->content), "http" );

    cut_end = strstr ( (char *)(buffer->content), request->host ) + strlen ( request->host );

    new_buffer_length = (void *)cut_beginning - buffer->content;
    new_buffer = malloc ( new_buffer_length );
    memmove ( new_buffer, buffer->content, new_buffer_length );

    new_buffer = realloc ( new_buffer, new_buffer_length +
                           buffer->content + buffer->length - (void *)cut_end );

    memmove ( new_buffer + new_buffer_length, (void *)cut_end,
              buffer->content + buffer->length - (void *)cut_end );

    new_buffer_length += buffer->content + buffer->length - (void *)cut_end;

    free ( buffer->content );

    buffer->content = new_buffer;
    buffer->length = new_buffer_length;

    return;
}
