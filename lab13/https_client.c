#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include "stdio.h"
#include "string.h"



void get_page(char *link, int port) {
    BIO *bio;
    SSL *ssl;
    SSL_CTX *ctx;

    int p;

    // char *request = "GET / HTTP/1.0\r\nHost: www.verisign.com\r\n\r\n";
    char request[50] = "GET / HTTP/1.0\r\nHost: ";
    strcat(request, link);
    strcat(request, "\r\n\r\n");

    char r[1024];

    /* initializare librarie */

    SSL_library_init();
    ERR_load_BIO_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    /* TO DO initializare context */
    ctx = SSL_CTX_new(SSLv23_client_method());

    /* incarca trust store */

    if (!SSL_CTX_load_verify_locations(ctx, "TrustStore.pem", NULL)) {
        fprintf(stderr, "Error loading trust store\n");
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return;
    }

    /* stabileste conexiune */

    bio = BIO_new_ssl_connect(ctx);

    /* Seteaza flag SSL_MODE_AUTO_RETRY  */

    BIO_get_ssl(bio, &ssl);
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    /* TO DO setup conexiune */

    // BIO *bio = BIO_new_ssl_connect(ctx);

    // BIO_set_conn_hostname(bio, "www.verisign.com:443");
    sprintf(link, ":%d", port);
    BIO_set_conn_hostname(bio, "www.verisign.com:443");

    if (BIO_do_connect(bio) <= 0) {
        perror("connection error");
    }

    /* TO DO verifica certificat */
    if (SSL_get_verify_result(ssl) != X509_V_OK) {
        perror("Certificatul nu este valid");
    }

    /* Trimite request */

    BIO_write(bio, request, strlen(request));

    /* TO DO Citeste raspuns si pregateste  output*/

    while (BIO_read(bio, r, strlen(r))) {
        printf("%s", r);
    }

    /* Inchide conexiune si elibereaza context */

    BIO_free_all(bio);
    SSL_CTX_free(ctx);
}


int main() {
    
    
    char link[] = "www.verisign.com";

    int port = 443;

    get_page(link, port);
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    // BIO *bio;
    // SSL *ssl;
    // SSL_CTX *ctx;

    // int p;
    // // char *request = "GET / HTTP/1.0\r\nHost: www.verisign.com\r\n\r\n";
    
    // char link[20] = "www.verisign.com";
    // char request[50] = "GET / HTTP/1.0\r\nHost: ";
    // strcat(request, link);
    // strcat(request, "\r\n\r\n");
    // puts(request);
    // char r[1024];

    // /* initializare librarie */

    // SSL_library_init();
    // ERR_load_BIO_strings();
    // SSL_load_error_strings();
    // OpenSSL_add_all_algorithms();

    // /* TO DO initializare context */
    // ctx = SSL_CTX_new(SSLv23_client_method());

    // /* incarca trust store */

    // if (!SSL_CTX_load_verify_locations(ctx, "TrustStore.pem", NULL)) {
    //     fprintf(stderr, "Error loading trust store\n");
    //     ERR_print_errors_fp(stderr);
    //     SSL_CTX_free(ctx);
    //     return 0;
    // }

    // /* stabileste conexiune */

    // bio = BIO_new_ssl_connect(ctx);

    // /* Seteaza flag SSL_MODE_AUTO_RETRY  */

    // BIO_get_ssl(bio, &ssl);
    // SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    // /* TO DO setup conexiune */

    // // BIO *bio = BIO_new_ssl_connect(ctx);
    // BIO_set_conn_hostname(bio, "www.google.com.com:443");
    // // BIO_set_conn_hostname(bio, "www.verisign.com:443");
    

    // if (BIO_do_connect(bio) <= 0) {
    //     perror("connection error");
    // }

    // /* TO DO verifica certificat */
    // if (SSL_get_verify_result(ssl) != X509_V_OK) {
    //     perror("Certificatul nu este valid");
    // }

    // /* Trimite request */

    // BIO_write(bio, request, strlen(request));

    // /* TO DO Citeste raspuns si pregateste  output*/

    // while (BIO_read(bio, r, sizeof(r))) {
    //     printf("%s", r);
    // }

    // /* Inchide conexiune si elibereaza context */

    // BIO_free_all(bio);
    // SSL_CTX_free(ctx);
    return 0;
}
