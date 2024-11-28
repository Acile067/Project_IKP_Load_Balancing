#pragma once
#ifndef THREAD_UTILS_H
#define THREAD_UTILS_H

#include <winsock.h>
#include <windows.h>
#include "../Common/queueLBtoWorker.h"
#include "networking_utils.h"

// Structure to hold server socket information for the thread
typedef struct {
    SOCKET serverSocket;
    fd_set* readSet;
    fd_set* exceptSet;
    size_t maxFd;
} ThreadParams;

extern int lastAssignedWorker;
extern QUEUEELEMENT* dequeued;
extern WorkerArray g_WorkerArray;

// Function declarations
DWORD WINAPI AcceptConnectionsThread(LPVOID lpParam);
DWORD WINAPI ProcessMessagesThread(LPVOID lpParam);
DWORD WINAPI SendMassagesToWorkersRoundRobin(LPVOID lpParam);

#endif // THREAD_UTILS_H