#include "load_balancer_socket.h"

// Initialize WSA and server socket
int initialize_server_socket(ServerSocket* server, int port) {
    WSADATA ws;
    if (WSAStartup(MAKEWORD(2, 2), &ws) < 0) {
        fprintf(stderr, "WSA Initialization Failed\n");
        return 0;
    }
    printf("WSA Initialized\n");

    server->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server->socket == INVALID_SOCKET) {
        fprintf(stderr, "Failed to create socket\n");
        WSACleanup();
        return 0;
    }
    printf("Socket created successfully\n");

    server->address.sin_family = AF_INET;
    server->address.sin_port = htons(port);
    server->address.sin_addr.s_addr = INADDR_ANY;
    memset(&(server->address.sin_zero), 0, 8);

    return 1;
}

// Bind and listen on the socket
int bind_and_listen(ServerSocket* server, int backlog) {
    if (bind(server->socket, (struct sockaddr*)&server->address, sizeof(server->address)) == SOCKET_ERROR) {
        fprintf(stderr, "Bind failed\n");
        cleanup_server_socket(server);
        return 0;
    }
    printf("Bind successful\n");

    if (listen(server->socket, backlog) == SOCKET_ERROR) {
        fprintf(stderr, "Listen failed\n");
        cleanup_server_socket(server);
        return 0;
    }
    printf("Listening on port %d\n", ntohs(server->address.sin_port));

    return 1;
}

// Cleanup server socket
void cleanup_server_socket(ServerSocket* server) {
    if (server->socket != INVALID_SOCKET) {
        closesocket(server->socket);
        server->socket = INVALID_SOCKET;
    }
    WSACleanup();
}
