// Load_Balancer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <winsock.h>
#include <windows.h>
#include "load_balancer_socket.h"
#include "networking_utils.h"
#include "thread_utils.h"
#include "init_resources.h"
#include "../Common/queueThreadPool.h"

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
    //////////////////////////

    // 1. Inicijalizacija reda sa kapacitetom 5
    THREAD_QUEUE* queue = init_thread_queue(5);

    // 2. Kreiranje dva elementa
    THREAD_QUEUEELEMENT* element1 = create_thread_queue_element("Client1", "Data1", 8080);
    THREAD_QUEUEELEMENT* element2 = create_thread_queue_element("Client2", "Data2", 9090);

    // 3. Dodavanje elemenata u red
    enqueue_thread_queue(queue, element1);
    enqueue_thread_queue(queue, element2);

    // 4. Ispis sadržaja reda
    cout << "Sadržaj reda nakon dodavanja elemenata:" << endl;
    print_thread_queue(queue);

    // 5. Uklanjanje jednog elementa iz reda
    THREAD_QUEUEELEMENT* dequeuedElement = dequeue_thread_queue(queue);

    // Provera i ispis uklonjenog elementa
    if (dequeuedElement) {
        cout << "Dequeued element:" << endl;
        cout << "ClientName: " << dequeuedElement->clientName
            << ", Data: " << dequeuedElement->data
            << ", TargetPort: " << dequeuedElement->targetPort << endl;
    }

    // 6. Ispis sadržaja reda nakon uklanjanja
    cout << "Sadrzaj reda nakon dequeue operacije:" << endl;
    print_thread_queue(queue);

    // 7. Oslobađanje memorije za red i elemente
    delete_thread_queue(queue);

    /////////////////////////

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
    HANDLE threadHandle = CreateThread(
        NULL,                  // Default security attributes
        0,                     // Default stack size
        AcceptConnectionsThread, // Thread function
        &params,               // Thread parameters
        0,                     // Default creation flags
        NULL                   // No thread ID needed
    );

    if (threadHandle == NULL) {
        fprintf(stderr, "Failed to create new connections thread\n");
        cleanup_server_socket(&server);                                 //From: load_balancer_socket.h
        return EXIT_FAILURE;
    }

    HANDLE hThread = CreateThread(
        NULL, 
        0, 
        ProcessMessagesThread, 
        NULL, 
        0, 
        NULL
    );

    if (hThread == NULL) {
        fprintf(stderr, "Failed to create msg processor thread\n");
        cleanup_server_socket(&server);                                 //From: load_balancer_socket.h
        return EXIT_FAILURE;
    }

    HANDLE h2Thread = CreateThread(
        NULL,
        0,
        SendMassagesToWorkersRoundRobin,
        NULL,
        0,
        NULL
    );

    if (h2Thread == NULL) {
        fprintf(stderr, "Failed to create msg processor thread\n");
        cleanup_server_socket(&server);                                 //From: load_balancer_socket.h
        return EXIT_FAILURE;
    }

    printf("Server is running. Press Ctrl+C to terminate.\n");

    // Wait for the thread to finish (infinite wait for this example)
    WaitForSingleObject(threadHandle, INFINITE);
    WaitForSingleObject(hThread, INFINITE);
    WaitForSingleObject(h2Thread, INFINITE);

    // Cleanup resources
    CloseHandle(threadHandle);
    CloseHandle(hThread);
    CloseHandle(h2Thread);

    cleanup_worker_array_critical_section();

    // Cleanup server socket
    WSACleanup();
    free_resources(&nClientWorkerSocketTable, &nClientMSGTable, &nClientMsgsQueue); //From: init_resources.h
    cleanup_server_socket(&server);
    return EXIT_SUCCESS;
}
