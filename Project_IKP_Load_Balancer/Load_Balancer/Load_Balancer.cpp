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
PORT_QUEUE* nWokrerPortQueue = NULL; //////////
WorkerArray g_WorkerArray;
CRITICAL_SECTION g_workerArrayCriticalSection;
int lastAssignedWorker = -1;
QUEUEELEMENT* dequeued;
PORT_QUEUEELEMENT* dequeuedPort; ///////////


int main()
{
    ServerSocket server;

    ////////////
    


    // 1. Inicijalizacija reda sa kapacitetom od 3 (možete promeniti kapacitet)
    PORT_QUEUE* queue = init_port_queue(3);
    if (queue == NULL) {
        printf("Greska pri inicijalizaciji reda.\n");
        return -1;
    }

    // 2. Kreiranje dva elementa reda
    uint16_t ports1[] = { 80, 443 };
    PORT_QUEUEELEMENT* element1 = create_port_queue_element("Klijent1", ports1, 2);

    uint16_t ports2[] = { 8080, 9090 };
    PORT_QUEUEELEMENT* element2 = create_port_queue_element("Klijent2", ports2, 2);

    if (element1 == NULL || element2 == NULL) {
        printf("Greska pri kreiranju elemenata.\n");
        // Oslobađanje memorije pre izlaska, samo ako je alociranje failovalo
        if (element1 != NULL) {
            free(element1->clientName);
            free(element1->ports);
            free(element1);
        }
        if (element2 != NULL) {
            free(element2->clientName);
            free(element2->ports);
            free(element2);
        }
        delete_port_queue(queue);  // Oslobađanje memorije reda
        return -1;
    }

    // 3. Dodavanje elemenata u red
    enqueue_port(queue, element1);
    enqueue_port(queue, element2);

    // 4. Ispisivanje sadržaja reda
    print_port_queue(queue);

    // 5. Uklanjanje jednog elementa (dequeue)
    dequeuedPort = dequeue_port(queue);
    if (dequeuedPort != NULL) {
        printf("Uklonjen element: %s\n", dequeuedPort->clientName);
    }

    // 6. Ponovno ispisivanje reda nakon dequeue
    print_port_queue(queue);

    // Oslobađanje memorije
    delete_port_queue(queue);  // Oslobađanje memorije reda (ovo treba da oslobodi sve što je alocirano za elemente)

    // Oslobađanje memorije za elemente - NE treba više da se oslobađa element1 i element2 ovde ako je delete_port_queue već oslobađao
    // Ako ste prethodno oslobodili u delete_port_queue, ne treba ponovno pozivati `free`



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
