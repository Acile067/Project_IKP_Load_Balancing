#include "client_socket.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//Init WSA
int initialize_socket(ClientSocket* client) {
    WSADATA ws;
    if (WSAStartup(MAKEWORD(2, 2), &ws) < 0) {
        fprintf(stderr, "WSA Initialization Failed\n");
        return 0;
    }

    client->socket = INVALID_SOCKET;
    memset(&client->server_address, 0, sizeof(client->server_address));
    return 1;
}

//Close Socket
void cleanup_socket(ClientSocket* client) {
    if (client->socket != INVALID_SOCKET) {
        closesocket(client->socket);
        client->socket = INVALID_SOCKET;
    }
    WSACleanup();
}

//connect to Load Balancer
//if ret = 1 = success   else ret = 0 = closesocket
int connect_to_server(ClientSocket* client, const char* ip_address, int port) {
    client->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client->socket < 0) {
        fprintf(stderr, "Failed to create socket\n");
        return 0;
    }

    client->server_address.sin_family = AF_INET;
    client->server_address.sin_port = htons(port);
    client->server_address.sin_addr.s_addr = inet_addr(ip_address);

    if (connect(client->socket, (struct sockaddr*)&client->server_address, sizeof(client->server_address)) < 0) {
        fprintf(stderr, "Failed to connect to server\n");
        cleanup_socket(client);
        return 0;
    }

    send(client->socket, "CLIENT", 6, 0);
    printf("Connected to server\n");
    return 1;
}

//send msg to Load Balancer
int send_message(ClientSocket* client, const char* message) {
    if (send(client->socket, message, strlen(message), 0) == SOCKET_ERROR) {
        fprintf(stderr, "Error sending message\n");
        return 0;
    }
    return 1;
}

//recv msg from Load Balancer
int receive_message(ClientSocket* client, char* buffer, size_t buffer_size) {
    int bytes_received = recv(client->socket, buffer, buffer_size - 1, 0);

    if (bytes_received > 0) {
        buffer[bytes_received] = '\0'; // Null-terminate the received data
        return 1;
    }

    return 0;
}
