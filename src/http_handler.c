#include "http_handler.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>
#include <sys/socket.h>

const char * get_mime_type(const char * path){
    if (strstr(path, ".html")) return "text/html";
    if (strstr(path, ".css")) return "text/css";
    if (strstr(path, ".js")) return "application/javascript";
    if (strstr(path, ".txt")) return "text/plain";
    if (strstr(path, ".c")) return "text/plain";
    if (strstr(path, ".png")) return "image/png";
    if (strstr(path, ".mp4")) return "video/mp4";
    return "application/octet-stream";
}


int file_exists(const char * fname){
    if (access(fname, F_OK) == 0) {
        return 1;
    } else {
        return 0;
    }
}

int is_directory(const char *path) {
    struct stat path_stat;
    if (stat(path, &path_stat) != 0) {
        // stat failed (e.g., path doesn't exist)
        return -1;
    }
    return S_ISDIR(path_stat.st_mode);
}

int is_file(const char *path) {
    struct stat path_stat;
    if (stat(path, &path_stat) != 0) {
        return -1;
    }
    return S_ISREG(path_stat.st_mode);
}

void format_size(char *out, size_t out_size, intmax_t bytes) {
    const char *units[] = { "B", "KB", "MB", "GB", "TB" };
    int i = 0;
    double size = (double)bytes;

    while (size >= 1024 && i < 4) {
        size /= 1024;
        i++;
    }

    snprintf(out, out_size, "%.1f %s", size, units[i]);
}


void serve_dir_listing(int new_fd, char * path,char * rel_path){
    DIR *dirp;
    struct dirent *dp;


    if ((dirp = opendir(path)) == NULL) {
        perror("couldn't open path");
        return;
    }

    dprintf(new_fd, "HTTP/1.1 200 OK\r\n");
    dprintf(new_fd, "Content-Type: text/html\r\n\r\n");

    dprintf(new_fd,
    "<html><style>td { padding: 8px; }</style><head><title>Index of Files</title></head><body><h1>Index of %s</h1><table>",rel_path);
    dprintf(new_fd,"<tr><td><a href=\"..\">..</a><br></td></tr>");

    
    do {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL) {
            if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) continue;
    
                
                char full_path[PATH_MAX];
                snprintf(full_path, sizeof(full_path), "%s/%s", path, dp->d_name);
                
                struct stat path_stat;
                stat(full_path, &path_stat);

                char human_size[32];
                format_size(human_size, sizeof(human_size), path_stat.st_size);

                char time_str[64];
                time_t mod_time = path_stat.st_mtime;
                struct tm *tm_info = localtime(&mod_time);
                strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M", tm_info);

                //  printf("Last file modification:   %s", ctime(&sb.st_mtime));
                if (S_ISREG(path_stat.st_mode)) {
                    dprintf(new_fd,
                        "<tr><td><a href=\"%s%s\">%s</a></td><td>%s</td><td>%s</td></tr>\n",
                        rel_path,dp->d_name, dp->d_name, human_size,time_str);
                } else {
                    dprintf(new_fd,
                        "<tr><td><a href=\"%s%s/\">%s</a></td><td>-</td><td>%s</td></tr>\n",
                        rel_path,dp->d_name ,dp->d_name,time_str);
                }
        }
    } while (dp != NULL);
        
    (void) closedir(dirp);
    dprintf(new_fd,"</table></body></html>");

    if (errno != 0){
        perror("error reading directory");
    }
        
    return;
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

    const char * mime = get_mime_type(filepath);
    char header[256];
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "\r\n"
        , mime, file_size);

    if(send(new_fd, header, strlen(header), 0) == -1){
        perror("send header");
        fclose(file_fd);
        return;
    }


    char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), file_fd)) > 0) {
        if (send(new_fd, buf, n, 0) == -1) {
            perror("send body");
            break;
     
        }
    }

    // bytes_read = fread(buffer,1,file_size,file_fd);
    fclose(file_fd);
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

        if(file_exists(file_path) && is_file(file_path)){
            serve_file(new_fd, file_path);
        }
        else {
            if(is_directory(file_path) == 1){
                // printf("%s is directory\n",file_path);
                serve_dir_listing(new_fd,file_path,path);
            }
            else{
                send_404(new_fd);
            }

            // serve_dir_listing(new_fd,file_path);
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