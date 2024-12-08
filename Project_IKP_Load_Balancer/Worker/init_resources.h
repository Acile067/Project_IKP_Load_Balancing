#pragma once
#ifndef INIT_RESOURCES_H
#define INIT_RESOURCES_H
#define QUEUESIZE 20

#include "../Common/hashtable.h"
#include "../Common/hashtablemsg.h"
#include "../Common/queueLBtoWorker.h"
#include "../Common/queueWorkerToWorker.h"
#include "../Common/queueThreadPool.h"

// Function to initialize all required resources
void initialize_resources(HASH_TABLE_MSG** msgTable, QUEUE** msgQueue, PORT_QUEUE** portQueue, THREAD_QUEUE** threadPoolQueue);
void free_resources(HASH_TABLE_MSG** msgTable, QUEUE** msgQueue, PORT_QUEUE** portQueue, THREAD_QUEUE** threadPoolQueue);
#endif // INIT_RESOURCES_H