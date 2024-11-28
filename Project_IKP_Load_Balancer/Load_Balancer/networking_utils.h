#pragma once
#ifndef NETWORKING_UTILS_H
#define NETWORKING_UTILS_H

#include <winsock.h>
#include <stdio.h>
#include <string.h>
#include "../Common/hashtable.h"
#include "../Common/queueLBtoWorker.h"

#define MAX_WORKERS 50
#define RECV_BUFFER_SIZE 256

typedef struct {
    SOCKET socket;
    uint16_t port;
} Worker;

typedef struct {
    Worker workers[MAX_WORKERS];
    int count;
} WorkerArray;

// External hash table declaration
extern HASH_TABLE* nClientWorkerSocketTable;
extern QUEUE* nClientMsgsQueue;
extern HASH_TABLE_MSG* nClientMSGTable;
extern WorkerArray g_WorkerArray;
extern CRITICAL_SECTION g_workerArrayCriticalSection;

// Function declaration
void initialize_worker_array_critical_section();
void cleanup_worker_array_critical_section();
void process_new_request(SOCKET clientSocket);
void ProcessNewMessageOrDisconnectWorker(int nWorkerSocket);
void ProcessNewMessage(int nClientSocket);
int serialize_hash_table(HASH_TABLE_MSG* table, char* buffer, size_t size);
int send_hash_table(SOCKET socket, HASH_TABLE_MSG* table);
int handle_worker_message(SOCKET workerSocket, char* message, WorkerArray* workers);
int add_worker_to_array(WorkerArray* array, SOCKET socket, uint16_t port);
int recv_and_handle_worker_message(SOCKET workerSocket, char* buffer, int bufferLength, WorkerArray* workers);
int remove_worker_from_array(WorkerArray* workers, SOCKET socket);
#endif // NETWORKING_UTILS_H