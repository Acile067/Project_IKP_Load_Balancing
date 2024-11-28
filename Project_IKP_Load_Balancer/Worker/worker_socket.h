#pragma once
#ifndef WORKER_SOCKET_H
#define WORKER_SOCKET_H

#include <winsock.h>
#include <stdint.h>

#define WORKER_MSG "WORKER"
#define BUFFER_SIZE 256
#define MAX_PENDING_CONNECTIONS SOMAXCONN

typedef struct WorkerSockets {
    SOCKET connectionSocket;  // Socket for connecting to LB
    SOCKET listeningSocket;   // Socket for listening to other workers
    uint16_t listeningPort;   // Port on which the worker is listening
} WorkerSockets;

// Function declarations
int initialize_winsock();
int create_and_connect_to_lb(SOCKET* workerSocket, const char* serverIp, uint16_t serverPort);
int create_listening_socket(SOCKET* listeningSocket, uint16_t* assignedPort);
int initialize_worker_sockets(WorkerSockets* workerSockets, const char* serverIp, uint16_t serverPort);
void cleanup_worker_sockets(WorkerSockets* workerSockets);

#endif // WORKER_SOCKET_H