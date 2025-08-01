#include "socket_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>




//function which casts a struct sockaddr.sin addr to sockaddr_in 
void *get_in_addr(struct sockaddr *sa){
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    else{
        return &(((struct sockaddr_in6*)sa)->sin6_addr);
    }

}

/*creates and binds the socket to the port, return the fd*/
int setup_server_socket(const char * port, int backlog){
    int sockfd;
    struct addrinfo hints, * servinfo,*p;
    int yes = 1;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if((rv = getaddrinfo(NULL,port,&hints,&servinfo)) != 0){
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

    if(listen(sockfd,backlog) == -1){
        perror("listen");
        exit(1);

    }

    return sockfd;

}

// use the sockfd to accept and create client sockets for client server comm
int accept_client(int sockfd){
    struct sockaddr_storage their_addr;
    char s[INET6_ADDRSTRLEN];
    socklen_t sin_size = sizeof(their_addr);
    int new_fd = accept(sockfd,(struct sockaddr *) &their_addr, &sin_size);

    if(new_fd == -1){
        perror("accept");
        return -1;
    }

    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *) &their_addr),s,sizeof(s));
    printf("server : got connection from %s\n", s);
    return new_fd;
}