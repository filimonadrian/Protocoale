#include "requests.h"
#include "helpers.h"
#include <arpa/inet.h>
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <stdio.h>
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <unistd.h>     /* read, write, close */

char *compute_get_request(char *host, char *url, char *query_params /*lasam pe null */,
                          char **cookies /*la 4 trebuie incluse*/, int cookies_count) {

    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // Step 2: add the host
    if (host != NULL) {
        sprintf(line, "Host: %s", host);
    }
    compute_message(message, line);

    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    /*Cookie : ....  de cookies count ori*/
    if (cookies != NULL) {
        for (int i = 0; i < cookies_count; i++) {
            sprintf(line, "Cookie: %s", cookies[i]);
            compute_message(message, line);
        }
    }
    // Step 4: add final new line
    compute_message(message, "");
    return message;
}

char *compute_post_request(char *host, char *url, char *content_type, char **body_data /*aici sunt datele*/,
                           int body_data_fields_count, char **cookies, int cookies_count) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    //user name = student&password = student;
    char *body_data_buffer = calloc(LINELEN, sizeof(char));

    /*body_data[0] = "a=b", body_data[0] = "b=c" , concatenez cu &*/
    /*content type = application ... */
    /*lenght = length(line1) + .. + (n - 1) (&) */

    // Step 1: write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);

    // Step 2: add the host
    if (host != NULL) {
        sprintf(line, "Host: %s", host);
    }
    compute_message(message, line);

    /* Step 3: add necessary headers (Content-Type and Content-Length are mandatory)
            in order to write Content-Length you must first compute the message size
    */

    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    int content_length = 0;
    for (int i = 0; i < body_data_fields_count; i++) {
        strcat(body_data_buffer, body_data[i]);
        if (i < body_data_fields_count - 1) {
            strcat(body_data_buffer, "&");
        }
        content_length += strlen(body_data[i]);
    }

    content_length += body_data_fields_count - 1;

    sprintf(line, "Content-Length: %d", content_length);
    compute_message(message, line);

    // Step 4 (optional): add cookies
    if (cookies != NULL) {
        for (int i = 0; i < cookies_count; i++) {
            sprintf(line, "Cookie: %s", cookies[i]);
            compute_message(message, line);
        }
    }
    // Step 5: add new line at end of header
    compute_message(message, "");

    // Step 6: add the actual payload data

    // memset(line, 0, LINELEN);
    compute_message(message, body_data_buffer);

    free(line);
    return message;
}
