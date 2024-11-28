#include "thread_utils.h"
#include "networking_utils.h"

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

    while (true) {
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