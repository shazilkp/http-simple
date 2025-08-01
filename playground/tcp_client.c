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
#define MAXDATASIZE 100 //max no of bytes we can recieve at once


void *get_in_addr(struct sockaddr *sa){
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    else{
        return &(((struct sockaddr_in6*)sa)->sin6_addr);
    }

}

int main(int argc, char *argv[]){
    int sockfd,numbytes;

    char buf[MAXDATASIZE];
    struct addrinfo hints,*servinfo,*p;

    int rv;
    char s[INET6_ADDRSTRLEN];

    if(argc != 2){
        fprintf(stderr,"usage: ./client hostname\n");
        return 1;
    }

    memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if((rv = getaddrinfo(argv[1],PORT,&hints,&servinfo)) != 0){
        fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(rv));
        return 1;
    }

    //loop and connect to first
    for(p = servinfo;p != NULL; p = p->ai_next) {
        if((sockfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1){
            perror("client: socket");
			continue;
        }


        inet_ntop(p->ai_family,get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof(s));
        printf("client : attempting to connect to %s\n",s);

        if(connect(sockfd,p->ai_addr,p->ai_addrlen) == -1){
            perror("client: connect");
            close(sockfd);
            continue;
        }
        break;
    }


    if(p == NULL){
        fprintf(stderr,"client: failed to connect\n");
        return 2;
    }


    inet_ntop(p->ai_family,get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof(s));
    printf("client: connected to %s\n",s);

    freeaddrinfo(servinfo);

    if((numbytes = recv(sockfd,buf,MAXDATASIZE-1,0)) == -1){
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';

    printf("client: recieved '%s' \n",buf);
    close(sockfd);
    return 0;


}