// Load_Balancer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <winsock.h>
#include <windows.h>
#include "../Common/hashtable.h"
#include"../Common/queueLBtoWorker.h"
using namespace std;
#define PORT 5059
#define QUEUESIZE 20

int nSocket;
struct sockaddr_in srv;
fd_set fr, fw, fe;  //Read,Write,Exception
int nMaxFd;

HASH_TABLE* nClientWorkerSocketTable;
HASH_TABLE_MSG* nClientWorkerMSGTable;

CRITICAL_SECTION cs;  // Dodaj kritičnu sekciju

QUEUE* nClientMsgsQueue;
QUEUEELEMENT* dequeued;

int lastAssignedWorker = -1;


void RedistributeDataToWorker(int nWorkerSocket)
{
    cout << "Redistributing data to worker: " << nWorkerSocket << endl;
  
    char ret[256 + 1];
    EnterCriticalSection(&nClientWorkerMSGTable->cs); // Zaštitite pristup hash tabeli
    convert_to_string(nClientWorkerMSGTable, ret, sizeof(ret));
    LeaveCriticalSection(&nClientWorkerMSGTable->cs);
    cout << "Converted nClientWorkerMSGTable: " << ret << endl;
    send(nWorkerSocket, ret, sizeof(ret), 0);
    
    cout << "Data successfully redistributed to worker." << endl;
}

void ProcessNewMessageOrDisconnectWorker(int nWorkerSocket)
{
    cout << endl << "Procesing message from worker: " << nWorkerSocket;
    char buffer[256 + 1] = { 0, };

    // Proveri da li je socket još uvek otvoren pre nego što pozoveš recv
    int nRet = recv(nWorkerSocket, buffer, 256, 0);
    if (nRet <= 0) {
        // Ako je socket zatvoren ili došlo do greške
        cout << endl << "Something bad happen. Closing Worker Socket " << nWorkerSocket << endl;
        closesocket(nWorkerSocket);

        EnterCriticalSection(&cs);
        LIST* workers = get_table_item(nClientWorkerSocketTable, "workers");
        if (workers != NULL && workers->count > 0) {
            LIST_ITEM* worker = workers->head;
            int nIndexCnt = 0;
            while (worker != NULL) {
                if (worker->data == nWorkerSocket) {
                    if (remove_from_list(workers, nIndexCnt)) {
                        print_hash_table(nClientWorkerSocketTable);
                    }
                    break; // Moze se prekinuti petlja nakon brisanja
                }
                nIndexCnt++;
                worker = worker->next;
            }
        }
        LeaveCriticalSection(&cs);
    }
    else 
    {
        cout << endl << "CANT HAPPEN NOW";
    }
}

void ProcessNewMessage(int nClientSocket) 
{
    cout << endl << "Procesing message from client: " << nClientSocket;
    char buffer[256 + 1] = { 0, };

    // Proveri da li je socket još uvek otvoren pre nego što pozoveš recv
    int nRet = recv(nClientSocket, buffer, 256, 0);
    if (nRet <= 0) {
        // Ako je socket zatvoren ili došlo do greške
        cout << endl << "Something bad happen. Closing Client Socket " << nClientSocket << endl;
        closesocket(nClientSocket);

        EnterCriticalSection(&cs);
        LIST* clients = get_table_item(nClientWorkerSocketTable, "clients");
        if (clients != NULL && clients->count > 0) {
            LIST_ITEM* client = clients->head;
            int nIndexCnt = 0;
            while (client != NULL) {
                if (client->data == nClientSocket) {
                    if (remove_from_list(clients, nIndexCnt)) {
                        print_hash_table(nClientWorkerSocketTable);
                    }
                    break; // Moze se prekinuti petlja nakon brisanja
                }
                nIndexCnt++;
                client = client->next;
            }
        }
        LeaveCriticalSection(&cs);
    }
    else {
        cout << endl << "CLIENT SAY: " << buffer;
        send(nClientSocket, "SERVER: Got message!", 21, 0);
        cout << endl << "******************";
        char hellper[50];
        snprintf(hellper, sizeof(hellper), "client-%d", nClientSocket);
        const char* clientName = hellper;
        cout << endl << clientName << endl;

        if (!get_table_item_msg(nClientWorkerMSGTable, clientName))
        {
            if (add_list_table_msg(nClientWorkerMSGTable, clientName)) {}
        }
        if (add_table_item_msg(nClientWorkerMSGTable, clientName, buffer)) {}

        print_hash_table_msg(nClientWorkerMSGTable);

        EnterCriticalSection(&cs);
        enqueue(nClientMsgsQueue, create_queue_element(clientName, buffer));
        LeaveCriticalSection(&cs);
    }
}

void ProcessTheNewRequest()
{
    // New Connection Request
    if (FD_ISSET(nSocket, &fr)) {
        int nLen = sizeof(struct sockaddr);
        int nClientSocket = accept(nSocket, NULL, &nLen);
        if (nClientSocket > 0) {
            char idBuffer[256] = { 0 };
            recv(nClientSocket, idBuffer, 256, 0);  // rcv client_hello or worker_hello

            if (strcmp(idBuffer, "CLIENT") == 0) {
                add_table_item(nClientWorkerSocketTable, "clients", nClientSocket);
                cout << "Added a client to the table with socket: " << nClientSocket << endl;
                send(nClientSocket, "SERVER: You are connected as CLIENT", 36, 0);
                print_hash_table(nClientWorkerSocketTable);
            }
            else if (strcmp(idBuffer, "WORKER") == 0) {
                add_table_item(nClientWorkerSocketTable, "workers", nClientSocket);
                cout << "Added a worker to the table with socket: " << nClientSocket << endl;
                send(nClientSocket, "SERVER: You are connected as WORKER", 36, 0);
                print_hash_table(nClientWorkerSocketTable);
                RedistributeDataToWorker(nClientSocket);
            }
            else {
                cout << "Unknown connection type" << endl;
                closesocket(nClientSocket);
            }
        }
    }
}

// Thread funkcija za prihvatanje novih konekcija
DWORD WINAPI AcceptConnectionsThread(LPVOID lpParam) {
    while (true) {
        FD_ZERO(&fr);
        FD_ZERO(&fe);
        FD_SET(nSocket, &fr);
        FD_SET(nSocket, &fe);

        struct timeval tv;
        tv.tv_sec = 1;
        int nRet = select(nMaxFd + 1, &fr, NULL, &fe, &tv);
        if (nRet > 0 && FD_ISSET(nSocket, &fr)) {
            ProcessTheNewRequest();
        }
    }
    return 0;
}

DWORD WINAPI ProcessMessagesThread(LPVOID lpParam) {
    while (true) {
        FD_ZERO(&fr);
        FD_ZERO(&fe);

        // Dodajemo sve klijente u fd_set za proveru poruka
        EnterCriticalSection(&cs);
        LIST* clients = get_table_item(nClientWorkerSocketTable, "clients");
        if (clients != NULL && clients->count > 0) {
            LIST_ITEM* client = clients->head;
            while (client != NULL) {
                SOCKET nClientMSGSocket = client->data;
                FD_SET(nClientMSGSocket, &fr);
                FD_SET(nClientMSGSocket, &fe);
                client = client->next;
            }
        }
        LeaveCriticalSection(&cs);

        EnterCriticalSection(&cs);
        LIST* workers = get_table_item(nClientWorkerSocketTable, "workers");
        if (workers != NULL && workers->count > 0) {
            LIST_ITEM* worker = workers->head;
            while (worker != NULL) {
                SOCKET nWorkerSocket = worker->data;
                FD_SET(nWorkerSocket, &fr); // Dodajemo u fd_set za workere
                FD_SET(nWorkerSocket, &fe); // Dodajemo u exception fd_set za workere
                worker = worker->next;
            }
        }
        LeaveCriticalSection(&cs);

        struct timeval tv;
        tv.tv_sec = 1;
        bool clientGotMessage = false;
        int nRet = select(nMaxFd + 1, &fr, NULL, &fe, &tv);
        if (nRet > 0) {
            EnterCriticalSection(&cs);
            LIST_ITEM* client = clients ? clients->head : nullptr;
            while (client != NULL) {
                if (client == NULL || client == reinterpret_cast<LIST_ITEM*>(0xdddddddddddddddd)) {
                    cout << "Detected invalid client pointer." << endl;
                    break;
                }
                SOCKET nClientMSGSocket = client->data;
                if (FD_ISSET(nClientMSGSocket, &fr)) {
                    ProcessNewMessage(nClientMSGSocket);
                    clientGotMessage = true;
                    //break;
                }
                client = client->next;
            }
            LeaveCriticalSection(&cs);

            if (!clientGotMessage)
            {
                EnterCriticalSection(&cs);
                LIST_ITEM* worker = workers ? workers->head : nullptr;
                while (worker != NULL) {
                    if (worker == NULL || worker == reinterpret_cast<LIST_ITEM*>(0xdddddddddddddddd)) {
                        cout << "Detected invalid worker pointer." << endl;
                        break;
                    }
                    SOCKET nWorkerSocket = worker->data;
                    if (FD_ISSET(nWorkerSocket, &fr)) {
                        ProcessNewMessageOrDisconnectWorker(nWorkerSocket);
                        //break;
                    }
                    worker = worker->next;
                }
                LeaveCriticalSection(&cs);
            }
        }
    }
    return 0;
}

void SendMsgToWorker(SOCKET nWorkerSocket, char* msg)
{
    send(nWorkerSocket, msg, (int)strlen(msg), 0); // Šalje poruku kroz soket
    cout << endl << nWorkerSocket << "->" << msg;
}

DWORD WINAPI SendMassagesToWorkersRoundRobin(LPVOID lpParam) {
    while (true) 
    {
        if (nClientMsgsQueue->currentSize > 0)
        {
            EnterCriticalSection(&cs);
            dequeued = dequeue(nClientMsgsQueue);
            LeaveCriticalSection(&cs);

            EnterCriticalSection(&cs);
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

                        char message[256]; // Ensure the buffer is large enough
                        snprintf(message, sizeof(message), "%s:%s", dequeued->clientName, dequeued->data);

                        SendMsgToWorker(nWorkerSocket, message);  // Send the data to this worker
                        break;
                    }
                    nIndex++;
                    worker = worker->next;
                }
            }
            LeaveCriticalSection(&cs);

        }
    }
}

int main()
{
    nClientWorkerSocketTable = init_hash_table();
    add_list_table(nClientWorkerSocketTable, "clients");
    add_list_table(nClientWorkerSocketTable, "workers");

    nClientWorkerMSGTable = init_hash_table_msg();

    nClientMsgsQueue = init_queue(QUEUESIZE);

    // Inicijalizacija kritične sekcije u main funkciji
    InitializeCriticalSection(&cs);

    // Inicijalizacija Winsock-a i kreiranje socket-a
    WSADATA ws;
    if (WSAStartup(MAKEWORD(2, 2), &ws) < 0) {
        cout << "WSA Failed" << endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    cout << "WSA Success" << endl;

    nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (nSocket < 0) {
        cout << "Socket not open" << endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    cout << "Socket open" << endl;

    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    srv.sin_addr.s_addr = INADDR_ANY;
    memset(&(srv.sin_zero), 0, 8);

    // Bind i listen
    bind(nSocket, (sockaddr*)&srv, sizeof(sockaddr));
    listen(nSocket, 20);
    cout << "Listen Ready:" << endl;

    // Inicijalizacija threadova za nove konekcije i poruke i slanje poruka ka workeru
    HANDLE hThread1 = CreateThread(NULL, 0, AcceptConnectionsThread, NULL, 0, NULL);
    HANDLE hThread2 = CreateThread(NULL, 0, ProcessMessagesThread, NULL, 0, NULL);
    HANDLE hThread3 = CreateThread(NULL, 0, SendMassagesToWorkersRoundRobin, NULL, 0, NULL);

    // Cekamo da thread-ovi završe (ili možete dodati dodatnu logiku za prekid)
    WaitForSingleObject(hThread1, INFINITE);
    WaitForSingleObject(hThread2, INFINITE);
    WaitForSingleObject(hThread3, INFINITE);

    // Zatvori socket i očisti Winsock
    DeleteCriticalSection(&cs);
    closesocket(nSocket);
    WSACleanup();
    return 0;
    
}
