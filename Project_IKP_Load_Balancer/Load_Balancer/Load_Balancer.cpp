// Load_Balancer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <winsock.h>
#include <windows.h>
#include "load_balancer_socket.h"
#include "networking_utils.h"
#include "thread_utils.h"
#include "init_resources.h"


#define PORT 5059
#define BACKLOG 20

HASH_TABLE* nClientWorkerSocketTable = NULL;
HASH_TABLE_MSG* nClientMSGTable = NULL;
QUEUE* nClientMsgsQueue = NULL;
WorkerArray g_WorkerArray;
CRITICAL_SECTION g_workerArrayCriticalSection;
int lastAssignedWorker = -1;
QUEUEELEMENT* dequeued;


int main()
{  
    ServerSocket server;

    initialize_worker_array_critical_section();
    
    initialize_resources(&nClientWorkerSocketTable, &nClientMSGTable, &nClientMsgsQueue);   //From: init_resources.h

    if (!nClientWorkerSocketTable || !nClientMSGTable || !nClientMsgsQueue) {
        fprintf(stderr, "Failed to initialize all resources\n");
        return EXIT_FAILURE;
    }

    // Initialize server socket
    if (!initialize_server_socket(&server, PORT)) {                     //From: load_balancer_socket.h
        fprintf(stderr, "Failed to initialize server socket\n");
        return EXIT_FAILURE;
    }

    // Bind and listen
    if (!bind_and_listen(&server, BACKLOG)) {                           //From: load_balancer_socket.h
        fprintf(stderr, "Failed to bind and listen on server socket\n");
        cleanup_server_socket(&server);                                 //From: load_balancer_socket.h
        return EXIT_FAILURE;
    }

    fd_set readSet, exceptSet;
    ThreadParams params;                                                //From: thread_utils.h
    params.serverSocket = server.socket;
    params.readSet = &readSet;
    params.exceptSet = &exceptSet;
    params.maxFd = (int)server.socket;

    // Create the thread for handling connections
    // Niz za handle-ove niti
    HANDLE threads[3];

    // Kreiranje niti za prihvatanje konekcija
    threads[0] = CreateThread(
        NULL,                   // Default security attributes
        0,                      // Default stack size
        AcceptConnectionsThread, // Thread function
        &params,                // Thread parameters
        0,                      // Default creation flags
        NULL                    // No thread ID needed
    );

    if (threads[0] == NULL) {
        fprintf(stderr, "Failed to create new connections thread\n");
        cleanup_server_socket(&server); // From: load_balancer_socket.h
        return EXIT_FAILURE;
    }

    // Kreiranje niti za procesiranje poruka
    threads[1] = CreateThread(
        NULL,
        0,
        ProcessMessagesThread,
        NULL,
        0,
        NULL
    );

    if (threads[1] == NULL) {
        fprintf(stderr, "Failed to create message processor thread\n");
        cleanup_server_socket(&server); // From: load_balancer_socket.h
        return EXIT_FAILURE;
    }

    // Kreiranje niti za slanje poruka radnicima (Round Robin)
    threads[2] = CreateThread(
        NULL,
        0,
        SendMassagesToWorkersRoundRobin,
        NULL,
        0,
        NULL
    );

    if (threads[2] == NULL) {
        fprintf(stderr, "Failed to create worker message thread\n");
        cleanup_server_socket(&server); // From: load_balancer_socket.h
        return EXIT_FAILURE;
    }

    printf("Server is running. Press Ctrl+C to terminate.\n");

    // Čekanje na sve niti
    WaitForMultipleObjects(3, threads, TRUE, INFINITE);
    //WaitForMultipleObjects(3, threads, TRUE, 180000);

    // Oslobađanje handle-ova za sve niti
    for (int i = 0; i < 3; i++) {
        CloseHandle(threads[i]);
    }

    cleanup_worker_array_critical_section();

    // Cleanup server socket
    WSACleanup();
    free_resources(&nClientWorkerSocketTable, &nClientMSGTable, &nClientMsgsQueue); //From: init_resources.h
    cleanup_server_socket(&server);
    return EXIT_SUCCESS;
}
