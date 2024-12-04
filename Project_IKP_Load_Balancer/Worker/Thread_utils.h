#pragma once
#ifndef THREAD_UTILS_H
#define THREAD_UTILS_H

#include <winsock.h>
#include <windows.h>
#include "../Common/queueWorkerToWorker.h"
#include "../Common/hashtable.h"
#include "../Common/hashtablemsg.h"

extern PORT_QUEUE* queueWithClientNameMsgPorts;
extern HASH_TABLE_MSG* nClientMSGTable;

DWORD WINAPI ProcessLBMessage(LPVOID lpParam);
DWORD WINAPI AcceptWorkerConnections(LPVOID lpParam);
DWORD WINAPI ConnectToWorkersAndSendMsg(LPVOID lpParam);

#endif // THREAD_UTILS_H