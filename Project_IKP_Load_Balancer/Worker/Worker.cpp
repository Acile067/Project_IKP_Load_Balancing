// Worker.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <winsock.h>
#include <sstream>
#include <cstring>
#include "worker_socket.h"
#include "init_resources.h"
#include "networking_utils.h"

HASH_TABLE_MSG* nClientMSGTable = NULL;
QUEUE* nClientMsgsQueue = NULL;

int main()
{
    WorkerSockets workerSockets;

    initialize_resources(&nClientMSGTable, &nClientMsgsQueue);                          //From: init_resources.h

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


    free_resources(&nClientMSGTable, &nClientMsgsQueue);                                //From: init_resources.h
    cleanup_worker_sockets(&workerSockets);
    return 0;
}

