// Load_Balancer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <winsock.h>
#include "../Common/hashtable.h"
using namespace std;
#define PORT 5059

int nSocket;
struct sockaddr_in srv;
fd_set fr, fw, fe;  //Read,Write,Exception
int nMaxFd;


HASH_TABLE* nClientWorkerSocketTable;
HASH_TABLE_MSG* nClientWorkerMSGTable;

void ProcessNewMessage(int nClientSocket) 
{
    cout << endl << "Procesing message from client: " << nClientSocket;
    char buffer[256 + 1] = { 0, };

    // Proveri da li je socket još uvek otvoren pre nego što pozoveš recv
    int nRet = recv(nClientSocket, buffer, 256, 0);
    if (nRet <= 0) {
        // Ako je socket zatvoren ili došlo do greške
        cout << endl << "Something bad happen. Closing Socket " << nClientSocket;

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
    }
    else {
        cout << endl << "CLIENT SAY: " << buffer;
        send(nClientSocket, "SERVER: Got message!", 21, 0);
        cout << endl << "******************";
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
            }
            else {
                cout << "Unknown connection type" << endl;
                closesocket(nClientSocket);
            }
        }
    }

    // Process all client sockets
    LIST* clients = get_table_item(nClientWorkerSocketTable, "clients");
    if (clients != NULL && clients->count > 0) {
        LIST_ITEM* client = clients->head;
        LIST_ITEM* prev = NULL;  // Keep track of the previous node
        int index = 0;

        while (client != NULL) {

            if (client == NULL || client == reinterpret_cast<LIST_ITEM*>(0xdddddddddddddddd)) {
                cout << "Detected invalid client pointer." << endl;
                break;
            }

            SOCKET nClientMSGSocket = client->data;

            // If socket is invalid, remove the client from the list
            if (nClientMSGSocket == INVALID_SOCKET) {
                cout << "Invalid socket detected. Removing client." << endl;

                // Remove the client node safely
                remove_from_list(clients, index);

                // Adjust pointers for the next iteration
                if (prev == NULL) {  // Removed head
                    client = clients->head;
                }
                else {
                    client = prev->next;
                }
                continue;  // Skip the rest and re-evaluate the new node
            }

            // Check if the socket is set in FD_SET and process it
            if (FD_ISSET(nClientMSGSocket, &fr)) {
                ProcessNewMessage(nClientMSGSocket);
            }

            // Move to the next client in the list
            prev = client;
            client = client->next;
            index++;
        }
    }
}

int main()
{
    nClientWorkerSocketTable = init_hash_table();
    
    if (add_list_table(nClientWorkerSocketTable, "clients")) {}
    if (add_list_table(nClientWorkerSocketTable, "workers")) {}

    //if (add_table_item(nClientWorkerSocketTable, "clients", 111)) {}

    print_hash_table(nClientWorkerSocketTable);
   


    nClientWorkerMSGTable = init_hash_table_msg();

    if (add_list_table_msg(nClientWorkerMSGTable, "client1")) {}
    if (add_table_item_msg(nClientWorkerMSGTable, "client1", "message1")) {}
    if (add_table_item_msg(nClientWorkerMSGTable, "client1", "message2")) {}

    print_hash_table_msg(nClientWorkerMSGTable);

    int nRet = 0;
    //Init WSA 
    WSADATA ws;
    if (WSAStartup(MAKEWORD(2, 2), &ws) < 0) 
    {
        cout << endl << "WSA Failed";
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << endl << "WSA Succes";
    }

    //Init Socket
    nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (nSocket < 0)
    {
        cout << endl << "Socket not open";
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << endl << "Socket open";
    }

    //Init env for sockaddr structure
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    srv.sin_addr.s_addr = INADDR_ANY;
    memset(&(srv.sin_zero), 0, 8);

    //Blocking or no blocking
    u_long optval = 0; //0 blocking !=0 non blocking
    nRet = ioctlsocket(nSocket, FIONBIO, &optval);
    if (nRet != 0)
    {
        cout << endl << "ioctlsocket call failed";
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << endl << "ioctlsocket call passed";
    }

    //setsockopt
    int nOptVal = 0;
    int nOptLen = sizeof(nOptVal);
    nRet = setsockopt(nSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&nOptVal, nOptLen);
    if (!nRet)
    {
        cout << endl << "setsockopt success";
    }
    else
    {
        cout << endl << "setsockopt failed";
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    //Binding
    nRet = bind(nSocket, (sockaddr*)&srv, sizeof(sockaddr));
    if (nRet < 0)
    {
        cout << endl << "Binding Failed";
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << endl << "Binding Succes";
    }

    //Listen for clients requests
    nRet = listen(nSocket, 20); //20 requests
    if (nRet < 0)
    {
        cout << endl << "Listen Failed";
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << endl << "Listen Succes";
    }

    //Keep waitnig for new clients
    nMaxFd = nSocket;
    struct timeval tv;
    tv.tv_sec = 1;

    while (1)
    {
        FD_ZERO(&fr);
        FD_ZERO(&fw);
        FD_ZERO(&fe);

        FD_SET(nSocket, &fr);
        FD_SET(nSocket, &fe);

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

        LIST* workers = get_table_item(nClientWorkerSocketTable, "workers");
        if (workers != NULL && workers->count > 0) {
            LIST_ITEM* worker = workers->head;
            while (worker != NULL) {
                SOCKET nWorkersSocket = worker->data;
                FD_SET(nWorkersSocket, &fr);
                FD_SET(nWorkersSocket, &fe);
                worker = worker->next;
            }
        }


        /*
        for (int nIndex = 0; nIndex < 5; nIndex++)           //TODO HashMap za ovo 5
        {
            if (nArrClient[nIndex] != 0)
            {
                FD_SET(nArrClient[nIndex], &fr);
                FD_SET(nArrClient[nIndex], &fe);
            }
        }*/
        //cout << endl << "Befor select call:" << fr.fd_count;

        nRet = select(nMaxFd + 1, &fr, &fw, &fe, &tv);
        if (nRet > 0)
        {
            cout << "NRET" << endl << nRet << endl << "NRET" << endl;
            //someone connected
            cout << endl << "Someone connected!";       //comment later
            //Process request
            ProcessTheNewRequest();

            //break;
        }
        else if (nRet == 0)
        {
            //no connections
            //cout << endl << "Nothing on Port";
        }
        else
        {
            //failed
            cout << endl << "Failed";
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        //cout << endl << "After select call:" << fr.fd_count;
        //Sleep(10000);
    }
    
}
