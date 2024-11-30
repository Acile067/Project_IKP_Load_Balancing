#pragma once
#ifndef INIT_RESOURCES_H
#define INIT_RESOURCES_H
#define QUEUESIZE 20

#include "../Common/hashtable.h"
#include "../Common/queueLBtoWorker.h"
#include "../Common/queueWorkerToWorker.h"

// Function to initialize all required resources
void initialize_resources(HASH_TABLE_MSG** msgTable, QUEUE** msgQueue, PORT_QUEUE** portQueue);
void free_resources(HASH_TABLE_MSG** msgTable, QUEUE** msgQueue, PORT_QUEUE** portQueue);
#endif // INIT_RESOURCES_H