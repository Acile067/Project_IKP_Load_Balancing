#pragma once
#ifndef THREAD_UTILS_H
#define THREAD_UTILS_H

#include <winsock.h>
#include <windows.h>

// Structure to hold server socket information for the thread
typedef struct {
    SOCKET serverSocket;
    fd_set* readSet;
    fd_set* exceptSet;
    size_t maxFd;
} ThreadParams;

// Function declarations
DWORD WINAPI AcceptConnectionsThread(LPVOID lpParam);
DWORD WINAPI ProcessMessagesThread(LPVOID lpParam);

#endif // THREAD_UTILS_H