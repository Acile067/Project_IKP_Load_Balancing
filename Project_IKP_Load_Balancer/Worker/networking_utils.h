#pragma once
#ifndef NETWORKING_UTILS_H
#define NETWORKING_UTILS_H

#include <winsock.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>  // For malloc and free
#include "../Common/hashtable.h"
#include "../Common/queueLBtoWorker.h"

#define MAX_TOKENS 100
#define MAX_TOKEN_LEN 255
#define BUFFER_SIZE 256

// External hash table declaration
extern QUEUE* nClientMsgsQueue;
extern HASH_TABLE_MSG* nClientMSGTable;

// Function declaration
int receive_hash_table(SOCKET socket, char* buffer, size_t size);
int split_string(const char* str, char delimiter, char output[MAX_TOKENS][MAX_TOKEN_LEN]);
int ParseFromStringToHashTable(char* data);
int receive_and_deserialize(SOCKET socket);
int send_worker_port(SOCKET socket, uint16_t port);


#endif // NETWORKING_UTILS_H