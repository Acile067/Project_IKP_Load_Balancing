#include "thread_utils.h"
#include "networking_utils.h"

extern PORT_QUEUE* queueWithClientNameMsgPorts;
extern HASH_TABLE_MSG* nClientMSGTable;

DWORD WINAPI ProcessLBMessage(LPVOID lpParam)
{
    // Preuzimanje socket-a iz lpParam
    SOCKET workerSocket = *(SOCKET*)lpParam;

    fd_set read_fds;           // Skup soketa za praćenje događaja za čitanje
    struct timeval timeout;    // Tajmaut za `select`
    int nRet;

    while (1)
    {
        FD_ZERO(&read_fds);
        FD_SET(workerSocket, &read_fds);

        // Postavljanje tajmauta (1 sekunda)
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        // Praćenje događaja na soketu
        nRet = select(0, &read_fds, NULL, NULL, &timeout);
        if (nRet > 0)
        {
            if (FD_ISSET(workerSocket, &read_fds)) {
                // Pozivanje funkcije za prijem i obradu podataka
                receive_combined_data(workerSocket);
            }
        }
        else if (nRet < 0) {
            // Obrada greške `select`
            printf("Error in select: %d\n", WSAGetLastError());
            break;
        }
    }

    return 0;
}

DWORD WINAPI AcceptWorkerConnections(LPVOID lpParam)
{
    SOCKET listeningSocket = *(SOCKET*)lpParam;

    while (1) {
        struct sockaddr_in workerAddr;
        int workerAddrSize = sizeof(workerAddr);

        SOCKET workerSocket = accept(listeningSocket, (struct sockaddr*)&workerAddr, &workerAddrSize);
        if (workerSocket == INVALID_SOCKET) {
            cout << "Error accepting Worker connection." << endl;
            continue;
        }

        cout << "New Worker connected: " << workerSocket << endl;

        
        char buffer[512];
        int bytesReceived = recv(workerSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            ClientMessage receivedMessage;
            if (deserialize_message(buffer, &receivedMessage) == 0) {
                printf("Received Message:\nClient Name: %s\nData: %s\n",
                    receivedMessage.clientName,
                    receivedMessage.data);


                if (!get_table_item_msg(nClientMSGTable, receivedMessage.clientName)) {
                    add_list_table_msg(nClientMSGTable, receivedMessage.clientName);
                }

                add_table_item_msg(nClientMSGTable, receivedMessage.clientName, receivedMessage.data);

                print_hash_table_msg(nClientMSGTable);

                // Oslobađanje memorije
                free(receivedMessage.clientName);
                free(receivedMessage.data);
            }
            else {
                printf("Failed to deserialize message\n");
            }
        }

        


        closesocket(workerSocket);
    }
}

DWORD WINAPI ConnectToWorkersAndSendMsg(LPVOID lpParam)
{
    uint16_t yourPort = *(uint16_t*)lpParam;
    
    while (1)
    {
        if (queueWithClientNameMsgPorts->currentSize > 0)
        {
            PORT_QUEUEELEMENT* dequeuedElement = dequeue_port(queueWithClientNameMsgPorts);

            for (int i = 0; i < dequeuedElement->numPorts; i++)
            {
                ProcessPort(dequeuedElement->ports[i], yourPort, dequeuedElement);
            }

            // Oslobađanje memorije zauzete redom
            free(dequeuedElement);
        }
        Sleep(1000);
    }
}