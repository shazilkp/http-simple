#ifndef SOCKET_HANDLER_H
#define SOCKET_HANDLER_H

int setup_server_socket(const char *port, int backlog);
int accept_client(int sockfd);

#endif