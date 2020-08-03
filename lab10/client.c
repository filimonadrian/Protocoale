#include "helpers.h"
#include "requests.h"
#include <arpa/inet.h>
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <unistd.h>     /* read, write, close */

int main(int argc, char *argv[]) {
    char *message;
    char *response;
    int sockfd;

    char **arr = malloc(100 * sizeof(char *));
    for (int i = 0; i < 100; i++) {
        arr[i] = malloc(100 * sizeof(char));
    }

    char **cookies = malloc(100 * sizeof(char *));
    for (int i = 0; i < 100; i++) {
        cookies[i] = malloc(100 * sizeof(char));
    }

    strcpy(arr[0], "username=student");
    strcpy(arr[1], "password=student");

// Ex 1.1: GET dummy from main server
    sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);


    message = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com",
                                  "/api/v1/dummy", NULL, NULL, 0);
    puts(message);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts(response);

// Ex 2: POST dummy and print response from main server


   // GET /api/v1/dummy HTTP/1.1
    // message = compute_post_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com",
    //                                "/api/v1/dummy", "application/x-www-form-urlencoded", arr, 2, NULL, 0);
    // puts(message);
    // send_to_server(sockfd, message);
    // response = receive_from_server(sockfd);
    // puts(response);

// Ex 3 --------login--------

    // message = compute_post_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com",
    //                                " /api/v1/auth/login", "application/x-www-form-urlencoded", arr, 2, NULL, 0);

    // puts(message);
    // send_to_server(sockfd, message);
    // response = receive_from_server(sockfd);
    // puts(response);

//Ex 4 -------key--------

    // strcpy(cookies[0], "connect.sid=s%3Ap2t6iH16XgvJsHncfM6dDSecwCeBiuW9.BhGhjQQpdQHWkhodkr5%2BBXr1Bnacko8l870Ksn8cd%2Fc");
    
    // message = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com",
    //                               "/api/v1/weather/key ", NULL, cookies, 1);
    // puts(message);
    // send_to_server(sockfd, message);
    // response = receive_from_server(sockfd);
    // puts(response);

    // {"key":"b912dd495585fbf756dc6d8f415a7649"}

//Ex 5 ------logout-------

    // strcpy(cookies[0], "connect.sid=s%3AAyQyj_vo5dwAws0q1isjGfIzIWnyI86F.9O6AKt6pIrYWUWU3XBzsYU0eM3vEyYJJdRwybs1x0Nc");
    // message = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com",
    //                               "/api/v1/auth/logout", NULL, cookies, 1);
    // puts(message);
    // send_to_server(sockfd, message);
    // response = receive_from_server(sockfd);
    // puts(response);

// BONUS: make the main server return "Already logged in!"

    // asta inseamna ca trebuie sa ma conectez din nou cu acelasi cookie, desi sunt conectat  
    // Already logged in!

    // strcpy(cookies[0], "connect.sid=s%3Ap2t6iH16XgvJsHncfM6dDSecwCeBiuW9.BhGhjQQpdQHWkhodkr5%2BBXr1Bnacko8l870Ksn8cd%2Fc");
    // message = compute_post_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com",
    //                                " /api/v1/auth/login", "application/x-www-form-urlencoded", arr, 2, cookies, 1);

    // puts(message);
    // send_to_server(sockfd, message);
    // response = receive_from_server(sockfd);
    // puts(response);

// BONUS: GET catre API-ul de la openweather

    //  i've used:  dig $api.openweathermap.org
    //;; ANSWER SECTION:
    // api.openweathermap.org.	1910	IN	A	188.166.16.132
    // api.openweathermap.org.	1910	IN	A	37.139.1.159
    // api.openweathermap.org.	1910	IN	A	37.139.20.5
    // api.openweathermap.org.	1910	IN	A	82.196.7.246

    /* create a new socket */
    sockfd = open_connection("37.139.20.5", 80, AF_INET, SOCK_STREAM, 0);

    /* cookie from the previous ex */
    // strcpy(cookies[0], "connect.sid=s%3Ap2t6iH16XgvJsHncfM6dDSecwCeBiuW9.BhGhjQQpdQHWkhodkr5%2BBXr1Bnacko8l870Ksn8cd%2Fc");

    // message = compute_get_request("api.openweathermap.org",
    //                               "/data/2.5/weather", "lat=45&lon=12&appid=b912dd495585fbf756dc6d8f415a7649", cookies, 1);
    // puts(message);
    // send_to_server(sockfd, message);
    // response = receive_from_server(sockfd);
    // puts(response);

    // output:
    /*

    GET /data/2.5/weather?lat=45&lon=12&appid=b912dd495585fbf756dc6d8f415a7649 HTTP/1.1
Host: api.openweathermap.org
Cookie: connect.sid=s%3Ap2t6iH16XgvJsHncfM6dDSecwCeBiuW9.BhGhjQQpdQHWkhodkr5%2BBXr1Bnacko8l870Ksn8cd%2Fc


HTTP/1.1 200 OK
Server: openresty
Date: Thu, 23 Apr 2020 16:49:24 GMT
Content-Type: application/json; charset=utf-8
Content-Length: 458
Connection: keep-alive
X-Cache-Key: /data/2.5/weather?lat=45&lon=12
Access-Control-Allow-Origin: *
Access-Control-Allow-Credentials: true
Access-Control-Allow-Methods: GET, POST

{"coord":{"lon":12,"lat":45},"weather":[{"id":800,"main":"Clear","description":
"clear sky","icon":"01d"}],"base":"stations","main":{"temp":294.06,"feels_like":290.33,
"temp_min":293.15,"temp_max":295.15,"pressure":1017,"humidity":26},"visibility":10000,"wind"
:{"speed":2.64,"deg":181},"clouds":{"all":0},"dt":1587660564,"sys":{"type":1,"id":6761,"country"
:"IT","sunrise":1587615188,"sunset":1587665221},"timezone":7200,"id":6534627,"name":"Papozze","cod":200}

*/

    // free the allocated data at the end!

    return 0;
}
