// Load_Balancer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <winsock.h>
#include <windows.h>
#include "load_balancer_socket.h"
#include "networking_utils.h"
#include "thread_utils.h"
#include "init_resources.h"


#include "../Common/queueWorkerToWorker.h" ////////

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

    ////////////

    cout << "||||||||||||||||||||||||||||||||||||||||||||||||||||||" << endl;
    
    // Kreiranje reda sa kapacitetom 5
    PORT_QUEUE* queue = init_port_queue(5);

    // Kreiranje i dodavanje prvog elementa
    uint16_t ports1[] = { 8080, 9090 };
    PORT_QUEUEELEMENT* element1 = create_port_queue_element("Client1", "Data1", ports1, 2);
    enqueue_port(queue, element1);

    // Kreiranje i dodavanje drugog elementa
    uint16_t ports2[] = { 1234, 5678 };
    PORT_QUEUEELEMENT* element2 = create_port_queue_element("Client2", "Data2", ports2, 2);
    enqueue_port(queue, element2);

    // Ispis trenutnog sadržaja reda
    cout << "Queue after enqueuing two elements:" << endl;
    print_port_queue(queue);

    // Dequeuing prvi element
    PORT_QUEUEELEMENT* dequeuedElement = dequeue_port(queue);
    if (dequeuedElement) {
        cout << "Dequeued element: " << dequeuedElement->clientName << ", " << dequeuedElement->data << endl;
    }

    // Ispis sadržaja reda nakon dequeue
    cout << "\nQueue after dequeuing one element:" << endl;
    print_port_queue(queue);

    // Uništavanje reda i oslobađanje memorije
    delete_port_queue(queue);

    cout << "||||||||||||||||||||||||||||||||||||||||||||||||||||||" << endl;


    ////////////

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
