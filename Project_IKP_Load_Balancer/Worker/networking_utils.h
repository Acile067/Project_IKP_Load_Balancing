#pragma once
#ifndef NETWORKING_UTILS_H
#define NETWORKING_UTILS_H

#include <winsock.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>  // For malloc and free
#include "../Common/hashtable.h"
#include "../Common/hashtablemsg.h"
#include "../Common/queueLBtoWorker.h"
#include "../Common/queueWorkerToWorker.h"

#define MAX_TOKENS 100
#define MAX_TOKEN_LEN 255
#define BUFFER_SIZE 256
#define MAX_WORKERS 50

// External hash table declaration
extern QUEUE* nClientMsgsQueue;
extern HASH_TABLE_MSG* nClientMSGTable;
extern PORT_QUEUE* queueWithClientNameMsgPorts;

typedef struct {
    SOCKET socket;
    uint16_t port;
} Worker;

typedef struct {
    Worker workers[MAX_WORKERS];
    int count;
} WorkerArray;

typedef struct {
    // Polja iz WorkerArray
    WorkerArray workerArray;

    // Polja koja čuvaju informacije o klijentu
    char* clientName;  // Ime klijenta
    char* data;        // Podaci koji se čuvaju u ovoj strukturi
} CombinedDataStructure;

typedef struct {
    char* clientName;
    char* data;
} ClientMessage;


// Function declaration
int receive_hash_table(SOCKET socket, char* buffer, size_t size);
int split_string(const char* str, char delimiter, char output[MAX_TOKENS][MAX_TOKEN_LEN]);
int ParseFromStringToHashTable(char* data);
int receive_and_deserialize(SOCKET socket);
int send_worker_port(SOCKET socket, uint16_t port);
int deserialize_combined_data_structure(char* buffer, int bufferSize, CombinedDataStructure* data);
void receive_combined_data(SOCKET serverSocket);
int serialize_message(const ClientMessage* message, char** buffer, int* size);
int deserialize_message(const char* buffer, ClientMessage* message);

#endif // NETWORKING_UTILS_H