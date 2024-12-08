// Worker.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <winsock.h>
#include <sstream>
#include <cstring>
#include "worker_socket.h"
#include "init_resources.h"
#include "networking_utils.h"
#include "thread_utils.h"
#include "../Common/queueThreadPool.h"

HASH_TABLE_MSG* nClientMSGTable = NULL;
QUEUE* nClientMsgsQueue = NULL;
PORT_QUEUE* queueWithClientNameMsgPorts = NULL;
THREAD_QUEUE* threadPoolQueue = NULL;

int main()
{
    WorkerSockets workerSockets;

    initialize_resources(&nClientMSGTable, 
                         &nClientMsgsQueue, 
                         &queueWithClientNameMsgPorts,
                         &threadPoolQueue
                                                    );                                 //From: init_resources.h

    if (initialize_worker_sockets(&workerSockets, "127.0.0.1", 5059) != 0) {            //From: worker_socket.h
        fprintf(stderr, "Failed to initialize worker sockets.\n");
        return -1;
    }

    printf("Worker is listening on port %u.\n", workerSockets.listeningPort);

    //recv client-123:msg1,msg2,msg3;client-456:hej; if hash table is empty recv: empty
    if (receive_and_deserialize(workerSockets.connectionSocket) == 0) {                 //From: networking_utils.h
        printf("Hash table received and deserialized successfully.\n");
        print_hash_table_msg(nClientMSGTable);
    }

    if (send_worker_port(workerSockets.connectionSocket, workerSockets.listeningPort) != 0) {
        fprintf(stderr, "Failed to send worker port to load balancer.\n");
        cleanup_worker_sockets(&workerSockets);
        return -1;
    }

    HANDLE threads[6];

    // Kreiranje glavnih niti
    threads[0] = CreateThread(
        NULL,
        0,
        ProcessLBMessage,
        &workerSockets.connectionSocket,
        0,
        NULL
    );

    if (threads[0] == NULL) {
        printf("Failed to create thread: %d\n", GetLastError());
        return -1;
    }

    threads[1] = CreateThread(
        NULL,
        0,
        AcceptWorkerConnections,
        &workerSockets.listeningSocket,
        0,
        NULL
    );

    if (threads[1] == NULL) {
        printf("Failed to create thread: %d\n", GetLastError());
        return -1;
    }

    threads[2] = CreateThread(
        NULL,
        0,
        ConnectToWorkersAndSendMsg,
        &workerSockets.listeningPort,
        0,
        NULL
    );

    if (threads[2] == NULL) {
        printf("Failed to create thread: %d\n", GetLastError());
        return -1;
    }

    // Kreiranje 3 dodatne niti za `ProcessPortTask`
    for (int i = 3; i < 6; i++) {
        threads[i] = CreateThread(NULL, 0, ProcessPortTask, NULL, 0, NULL);
        if (threads[i] == NULL) {
            printf("Failed to create thread %d: %d\n", i, GetLastError());
            return -1;
        }
    }

    // Čekanje na sve niti
    WaitForMultipleObjects(6, threads, TRUE, INFINITE);

    // Zatvaranje handle-ova za sve niti
    for (int i = 0; i < 6; i++) {
        CloseHandle(threads[i]);
    }

    free_resources(&nClientMSGTable, &nClientMsgsQueue, &queueWithClientNameMsgPorts, &threadPoolQueue);                //From: init_resources.h
    cleanup_worker_sockets(&workerSockets);
    return 0;
}

