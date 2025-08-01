#include "socket_handler.h"
#include "http_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

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

// //function which casts a struct sockaddr.sin addr to sockaddr_in 
// void *get_in_addr(struct sockaddr *sa){
//     if(sa->sa_family == AF_INET){
//         return &(((struct sockaddr_in*)sa)->sin_addr);
//     }
//     else{
//         return &(((struct sockaddr_in6*)sa)->sin6_addr);
//     }

// }
int main(){

    int sockfd = setup_server_socket(PORT,BACKLOG);
    
    struct sigaction sa;
    sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

    printf("server: waiting for connections...\n");

    while(1){       //working loop
        int new_fd = accept_client(sockfd);

        if(new_fd == -1){
            continue;
        }
        
        if(fork() == 0){            //child 
            close(sockfd);      //child dont need lstner

            handle_client_http(new_fd);

            close(new_fd);
            exit(0);
        }
        close(new_fd);

    }
    return 0;
}