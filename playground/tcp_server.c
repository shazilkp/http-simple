#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>


#define PORT "3480"     //string???

#define BACKLOG 10      //server queue size


/*

getaddrinfo();          to get port details
socket();               create socket with the details from getaddrinfo
setsockopt();           for setting option to allow immediate restart
bind();                 bind our port to this particular socket
accept();               accept from listening socket and provide new sockets for connection
fork() & send();        fork a process and let it handle th new connection

*/
void sigchld_handler(int s){
    (void)s;

    int saved_err = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_err;
}

void *get_in_addr(struct sockaddr *sa){
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    else{
        return &(((struct sockaddr_in6*)sa)->sin6_addr);
    }

}

int main(){
    int sockfd,new_fd;      //sockfd initial listener, new fd the one communiaction
    struct addrinfo hints, * servinfo,*p;

    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;        //static ip

    if((rv = getaddrinfo(NULL,PORT,&hints,&servinfo)) != 0){
        fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(rv));
        return 1;
    }

    //loop and bind
    for(p = servinfo;p != NULL; p = p->ai_next) {
        if((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1){
            perror("server: socket");
            continue;
        }

        //set option for our socket to available for reuse immediatly
        if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1){
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd,p->ai_addr, p->ai_addrlen) == -1){
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if(p == NULL){
        fprintf(stderr,"server : failed to bind\n");
        exit(1);
    }

    if(listen(sockfd,BACKLOG) == -1){
        perror("listen");
        exit(1);

    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

    printf("server: waiting for connections...\n");

    while(1){       //working loop
        sin_size = sizeof(their_addr);
        new_fd = accept(sockfd,(struct sockaddr *) &their_addr, &sin_size);

        if(new_fd == -1){
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *) &their_addr),s,sizeof(s));
        printf("server : got connection from %s\n", s);

        if(fork() == 0){            //child process
            close(sockfd);      //child dont need lstner

            

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
            // printf("Child (%d) : data sent\n",getpid());
            close(new_fd);
            exit(0);
        }
        close(new_fd);

    }

   

    return 0;
}