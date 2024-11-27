#pragma once
#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>

// Structure to manage the server socket
typedef struct {
    SOCKET socket;
    struct sockaddr_in address;
} ServerSocket;

// Function declarations
int initialize_server_socket(ServerSocket* server, int port);
void cleanup_server_socket(ServerSocket* server);
int bind_and_listen(ServerSocket* server, int backlog);

#endif // SERVER_SOCKET_H
