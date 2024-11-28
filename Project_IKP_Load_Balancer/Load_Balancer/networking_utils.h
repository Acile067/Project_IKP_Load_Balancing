#pragma once
#ifndef NETWORKING_UTILS_H
#define NETWORKING_UTILS_H

#include <winsock.h>
#include <stdio.h>
#include <string.h>
#include "../Common/hashtable.h"
#include "../Common/queueLBtoWorker.h"

// External hash table declaration
extern HASH_TABLE* nClientWorkerSocketTable;

// Function declaration
void process_new_request(SOCKET clientSocket);
void ProcessNewMessageOrDisconnectWorker(int nWorkerSocket);
void ProcessNewMessage(int nClientSocket);
#endif // NETWORKING_UTILS_H