#include "thread_utils.h"
#include "networking_utils.h"
#include <string.h>

extern int lastAssignedWorker;
extern QUEUEELEMENT* dequeued;
extern WorkerArray g_WorkerArray;

DWORD WINAPI AcceptConnectionsThread(LPVOID lpParam) {
    ThreadParams* params = (ThreadParams*)lpParam;

    SOCKET serverSocket = params->serverSocket;
    fd_set* readSet = params->readSet;
    fd_set* exceptSet = params->exceptSet;
    int maxFd = params->maxFd;

    while (1) {
        FD_ZERO(readSet);
        FD_ZERO(exceptSet);
        FD_SET(serverSocket, readSet);
        FD_SET(serverSocket, exceptSet);

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int selectResult = select(maxFd + 1, readSet, NULL, exceptSet, &tv);
        if (selectResult > 0 && FD_ISSET(serverSocket, readSet)) {
            struct sockaddr_in clientAddr;
            int clientAddrSize = sizeof(clientAddr);

            SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
            if (clientSocket != INVALID_SOCKET) {
                printf("New client/worker connected\n");

                // Process the new client request
                process_new_request(clientSocket);
            }
            else {
                fprintf(stderr, "Failed to accept connection\n");
            }
        }
    }

    return 0;
}


DWORD WINAPI ProcessMessagesThread(LPVOID lpParam) {
    fd_set fr, fe;
    int nMaxFd = 0; // Maksimalni socket ID za `select`

    while (1) {
        FD_ZERO(&fr);
        FD_ZERO(&fe);

        LIST* clients = get_table_item(nClientWorkerSocketTable, "clients");
        if (clients != NULL) {
            LIST_ITEM* client = clients->head;
            while (client != NULL) {
                SOCKET nClientMSGSocket = client->data;
                FD_SET(nClientMSGSocket, &fr);
                FD_SET(nClientMSGSocket, &fe);
                nMaxFd = max(nMaxFd, nClientMSGSocket);
                client = client->next;
            }
        }

        LIST* workers = get_table_item(nClientWorkerSocketTable, "workers");
        if (workers != NULL) {
            LIST_ITEM* worker = workers->head;
            while (worker != NULL) {
                SOCKET nWorkerSocket = worker->data;
                FD_SET(nWorkerSocket, &fr);
                FD_SET(nWorkerSocket, &fe);
                nMaxFd = max(nMaxFd, nWorkerSocket);
                worker = worker->next;
            }
        }

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int nRet = select(nMaxFd + 1, &fr, NULL, &fe, &tv);
        if (nRet > 0) {
            if (clients != NULL) {
                LIST_ITEM* client = clients->head;
                while (client != NULL) {
                    if (client == NULL || client == reinterpret_cast<LIST_ITEM*>(0xdddddddddddddddd)) {
                        cout << "Detected invalid client pointer." << endl;
                        break;
                    }
                    SOCKET nClientMSGSocket = client->data;
                    if (FD_ISSET(nClientMSGSocket, &fr)) {
                        ProcessNewMessage(nClientMSGSocket);
                    }
                    client = client->next;
                }
            }
            if (workers != NULL) {
                LIST_ITEM* worker = workers->head;
                while (worker != NULL) {
                    if (worker == NULL || worker == reinterpret_cast<LIST_ITEM*>(0xdddddddddddddddd)) {
                        cout << "Detected invalid worker pointer." << endl;
                        break;
                    }
                    SOCKET nWorkerSocket = worker->data;
                    if (FD_ISSET(nWorkerSocket, &fr)) {
                        ProcessNewMessageOrDisconnectWorker(nWorkerSocket);
                    }
                    worker = worker->next;
                }
            }
        }
    }
    return 0;
}

DWORD WINAPI SendMassagesToWorkersRoundRobin(LPVOID lpParam) {
    while (1)
    {
        if (nClientMsgsQueue != nullptr && nClientMsgsQueue->currentSize > 0)
        {
            dequeued = dequeue(nClientMsgsQueue);

            LIST* workers = get_table_item(nClientWorkerSocketTable, "workers");
            if (workers != NULL && workers->count > 0) {
                lastAssignedWorker = (lastAssignedWorker + 1) % workers->count;
                LIST_ITEM* worker = workers->head;
                int nIndex = 0;
                while (worker != NULL) {
                    if (nIndex == lastAssignedWorker) {
                        // Assign the task to the next worker
                        int nWorkerSocket = worker->data;
                        cout << "[Load Balancer] Assigning client request to worker: " << nWorkerSocket << endl;

                        CombinedDataStructure combinedData;
                        initialize_combined_data_structure(&combinedData);

                        // Popunjavanje klijentovih podataka
                        combinedData.clientName = _strdup(dequeued->clientName); // Kopira ime klijenta
                        combinedData.data = _strdup(dequeued->data);             // Kopira podatke klijenta

                        // Popunjavanje radnika
                        memcpy(&combinedData.workerArray, &g_WorkerArray, sizeof(WorkerArray));

                        char buffer[1024];
                        int serializedSize = serialize_combined_data_structure(&combinedData, buffer, sizeof(buffer));
                        if (serializedSize <= 0) {
                            printf("Failed to serialize data structure\n");
                            cleanup_combined_data_structure(&combinedData);
                            break;
                        }

                        if (send(nWorkerSocket, buffer, serializedSize, 0) == SOCKET_ERROR) {
                            printf("Failed to send message to worker %d\n", nWorkerSocket);
                        }
                        else {
                            cout << "[Load Balancer] Successfully sent message to worker: " << nWorkerSocket << endl;
                        }

                        // Čišćenje strukture
                        cleanup_combined_data_structure(&combinedData);
                        break;
                    }
                    nIndex++;
                    worker = worker->next;
                }
            }
        }
    }
}