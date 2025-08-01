#include "http_handler.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>


const char * get_mime_type(const char * path){
    if (strstr(path, ".html")) return "text/html";
    if (strstr(path, ".css")) return "text/css";
    if (strstr(path, ".js")) return "application/javascript";
    if (strstr(path, ".txt")) return "text/plain";
    if (strstr(path, ".png")) return "image/png";
    return "application/octet-stream";
}


int file_exists(const char * fname){
    if (access(fname, F_OK) == 0) {
        return 1;
    } else {
        return 0;
    }
}



void send_404(int new_fd){
    const char *body_404 =
        "<!DOCTYPE html>\r\n"
        "<html lang=\"en\">\r\n"
        "<head><meta charset=\"UTF-8\">\r\n"
        "<title>404 Not Found</title></head>\r\n"
        "<body><h1>404 - Not Found</h1>\r\n"
        "<p>The requested resource was not found on this server.</p>\r\n"
        "</body></html>";

    char response_404[1024];
    snprintf(response_404, sizeof(response_404),
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %lu\r\n"
        "\r\n"
        "%s", strlen(body_404), body_404);
    
    if(send(new_fd, response_404, strlen(response_404), 0) == -1){
        perror("send 404");
    }
}

void serve_file(int new_fd, const char * filepath){
    FILE * file_fd = fopen(filepath,"rb");

    if(file_fd == NULL){
        //404
        send_404(new_fd);
        return;
    }

    fseek(file_fd,0,SEEK_END);
    long file_size = ftell(file_fd);
    rewind(file_fd);

    ssize_t bytes_read;
    
    
    char *buffer = malloc(file_size);
    if (!buffer) {
        fclose(file_fd);
        perror("malloc");
        return;
    }

    bytes_read = fread(buffer,1,file_size,file_fd);
    fclose(file_fd);

    // if((bytes_read = read(file_fd,buffer,sizeof(buffer))) == -1){
    //     perror("file read");
    //     close(file_fd);
    //     return;
    // }

    // close(file_fd);
    const char * mime = get_mime_type(filepath);
    char header[256];
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "\r\n"
        , mime, bytes_read);

    if(send(new_fd, header, strlen(header), 0) == -1){
        perror("send header");
    }
    if(send(new_fd, buffer, bytes_read, 0) == -1){
        perror("send content");
    }

    free(buffer);
}



//handles the http request from client and sends a appropriate http response
void handle_client_http(int new_fd){
    char req_buf[1024];
    char method[8], path[256], version[16];

    int numbytes = recv(new_fd, req_buf, sizeof(req_buf) - 1, 0);

    if (numbytes > 0) {
        req_buf[numbytes] = '\0';
        printf("Received request:\n%s\n", req_buf);
        sscanf(req_buf,"%s %s %s",method,path,version);         //parsing the first line of http request
    }


    //serve according to path
    if (strcmp(path, "/") == 0) {
        serve_file(new_fd, "static/index.html");
    } else if (strcmp(path, "/about") == 0) {
        serve_file(new_fd, "static/about.html");
    } else {
        char file_path[256];
        snprintf(file_path, sizeof(file_path), "static%s", path);
        if (file_exists(file_path)) {
            serve_file(new_fd, file_path);
        } else {
            send_404(new_fd);
        }
    }

    
    // const char *body = "<body><h1>Networking</h1></body>";
    // char response[512];

    // snprintf(response, sizeof(response),
    //     "HTTP/1.1 200 OK\r\n"
    //     "Content-Type: text/html\r\n"
    //     "Content-Length: %ld\r\n"
    //     "\r\n"
    //     "%s", strlen(body), body);
    
    // if(send(new_fd, response, strlen(response), 0) == -1){
    //     perror("send");
    // }
}