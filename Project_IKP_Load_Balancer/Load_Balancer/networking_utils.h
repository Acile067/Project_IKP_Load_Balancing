#pragma once
#ifndef NETWORKING_UTILS_H
#define NETWORKING_UTILS_H

#include <winsock.h>
#include <stdio.h>
#include <string.h>
#include "../Common/hashtable.h"

// External hash table declaration
extern HASH_TABLE* nClientWorkerSocketTable;

// Function declaration
void process_new_request(SOCKET clientSocket);

#endif // NETWORKING_UTILS_H