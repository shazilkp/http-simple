#include "http_handler.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>


//handles the http request from client and sends a appropriate http response
void handle_client_http(int new_fd){
    char req_buf[1024];
    char method[8], path[256], version[16];

    int numbytes = recv(new_fd, req_buf, sizeof(req_buf) - 1, 0);

    if (numbytes > 0) {
        req_buf[numbytes] = '\0';
        printf("Received request:\n%s\n", req_buf);
        sscanf(req_buf,"%s %s %s",method,path,version);         //parsing the first line of http request
        printf("Method: %s\n", method);    // "GET"
        printf("Path: %s\n", path);        // "/index.html"
        printf("Version: %s\n", version);  // "HTTP/1.1"
    }


    const char *body = "<body><h1>Networking</h1></body>";
    char response[512];

    snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %ld\r\n"
        "\r\n"
        "%s", strlen(body), body);
    
    if(send(new_fd, response, strlen(response), 0) == -1){
        perror("send");
    }
}