#include "worker_socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Initialize the Winsock library
int initialize_winsock() {
    WSADATA ws;
    if (WSAStartup(MAKEWORD(2, 2), &ws) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        return -1;
    }
    printf("WSA initialized successfully.\n");
    return 0;
}

// Create and connect the worker socket to the load balancer
int create_and_connect_to_lb(SOCKET* workerSocket, const char* serverIp, uint16_t serverPort) {
    struct sockaddr_in serverAddr;

    *workerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (*workerSocket == INVALID_SOCKET) {
        fprintf(stderr, "Failed to create socket for LB connection.\n");
        WSACleanup();
        return -1;
    }

    printf("Worker connection socket created.\n");

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr(serverIp);

    if (connect(*workerSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        fprintf(stderr, "Failed to connect to load balancer.\n");
        closesocket(*workerSocket);
        WSACleanup();
        return -1;
    }

    printf("Connected to load balancer.\n");
    send(*workerSocket, WORKER_MSG, (int)strlen(WORKER_MSG), 0);
    printf("Sent WORKER message to load balancer.\n");

    char buffer[BUFFER_SIZE] = { 0 };
    recv(*workerSocket, buffer, BUFFER_SIZE - 1, 0);
    printf("[Load balancer]: %s\n", buffer);

    return 0;
}

// Create a listening socket for worker-to-worker communication
int create_listening_socket(SOCKET* listeningSocket, uint16_t* assignedPort) {
    struct sockaddr_in listeningAddr;
    int addrLen = sizeof(listeningAddr);

    *listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (*listeningSocket == INVALID_SOCKET) {
        fprintf(stderr, "Failed to create listening socket.\n");
        WSACleanup();
        return -1;
    }

    printf("Listening socket created.\n");

    listeningAddr.sin_family = AF_INET;
    listeningAddr.sin_port = 0; // Let the system choose a free port
    listeningAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(*listeningSocket, (struct sockaddr*)&listeningAddr, sizeof(listeningAddr)) == SOCKET_ERROR) {
        fprintf(stderr, "Failed to bind listening socket.\n");
        closesocket(*listeningSocket);
        WSACleanup();
        return -1;
    }

    if (listen(*listeningSocket, MAX_PENDING_CONNECTIONS) == SOCKET_ERROR) {
        fprintf(stderr, "Failed to listen on the socket.\n");
        closesocket(*listeningSocket);
        WSACleanup();
        return -1;
    }

    getsockname(*listeningSocket, (struct sockaddr*)&listeningAddr, (int*)&addrLen);
    *assignedPort = ntohs(listeningAddr.sin_port);

    printf("Listening socket bound to port %u.\n", *assignedPort);

    return 0;
}

// Initialize the worker sockets
int initialize_worker_sockets(WorkerSockets* workerSockets, const char* serverIp, uint16_t serverPort) {
    if (initialize_winsock() != 0) {
        return -1;
    }

    if (create_and_connect_to_lb(&workerSockets->connectionSocket, serverIp, serverPort) != 0) {
        return -1;
    }

    if (create_listening_socket(&workerSockets->listeningSocket, &workerSockets->listeningPort) != 0) {
        closesocket(workerSockets->connectionSocket);
        WSACleanup();
        return -1;
    }

    return 0;
}

// Clean up sockets
void cleanup_worker_sockets(WorkerSockets* workerSockets) {
    if (workerSockets->connectionSocket != INVALID_SOCKET) {
        closesocket(workerSockets->connectionSocket);
    }
    if (workerSockets->listeningSocket != INVALID_SOCKET) {
        closesocket(workerSockets->listeningSocket);
    }
    WSACleanup();
    printf("Cleaned up worker sockets and Winsock.\n");
}
