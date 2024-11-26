#pragma once
#ifndef CLIENT_SOCKET_H
#define CLIENT_SOCKET_H

#include <winsock.h>

// Struktura koja predstavlja klijentski soket
typedef struct {
    int socket;
    struct sockaddr_in server_address;
} ClientSocket;

// Deklaracija funkcija
int initialize_socket(ClientSocket* client);                                        
int connect_to_server(ClientSocket* client, const char* ip_address, int port);      
int send_message(ClientSocket* client, const char* message);
int receive_message(ClientSocket* client, char* buffer, size_t buffer_size);
void cleanup_socket(ClientSocket* client);

#endif // CLIENT_SOCKET_H
