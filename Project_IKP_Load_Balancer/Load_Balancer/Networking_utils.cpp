#include "networking_utils.h"

extern HASH_TABLE* nClientWorkerSocketTable;
extern QUEUE* nClientMsgsQueue;
extern HASH_TABLE_MSG* nClientMSGTable;
// Function to process a new client/worker request
void process_new_request(SOCKET clientSocket) {
    char buffer[256] = { 0 };

    // Receive initial message from the client
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0'; // Null-terminate the received string
        
        if (strcmp(buffer, "CLIENT") == 0) {
            printf("[Client]: %s\n", buffer);
            const char* response = "You are connected as CLIENT";
            send(clientSocket, response, strlen(response), 0);

            // Add client to the hash table under the "clients" key
            if (!add_table_item(nClientWorkerSocketTable, "clients", clientSocket)) {
                fprintf(stderr, "Failed to add client socket to the hash table\n");
            }
            else {
                printf("Client socket added to hash table\n");
                print_hash_table(nClientWorkerSocketTable);
            }
        }
        else if (strcmp(buffer, "WORKER") == 0) {
            printf("[Worker]: %s\n", buffer);
            const char* response = "You are connected as WORKER";
            send(clientSocket, response, strlen(response), 0);

            // Add client to the hash table under the "workers" key
            if (!add_table_item(nClientWorkerSocketTable, "workers", clientSocket)) {
                fprintf(stderr, "Failed to add worker socket to the hash table\n");
            }
            else {
                printf("Worker socket added to hash table\n");
                print_hash_table(nClientWorkerSocketTable);
            }
        }
        else {
            const char* response = "Unknown connection type";
            send(clientSocket, response, strlen(response), 0);
        }
    }
    else {
        fprintf(stderr, "Failed to receive data from client\n");
    }
}

void ProcessNewMessageOrDisconnectWorker(int nWorkerSocket) {
    printf("\nProcessing message from worker: %d\n", nWorkerSocket);
    char buffer[257] = { 0 }; // Dodat jedan bajt za null terminator

    // Provera stanja socket-a pre čitanja
    int nRet = recv(nWorkerSocket, buffer, 256, 0);
    if (nRet <= 0) {
        // Zatvaranje socket-a ako je prekinuta veza ili doslo do greske
        printf("\nSomething bad happened. Closing Worker Socket %d\n", nWorkerSocket);
        closesocket(nWorkerSocket);

        LIST* workers = get_table_item(nClientWorkerSocketTable, "workers");
        if (workers != NULL && workers->count > 0) {
            LIST_ITEM* worker = workers->head;
            int nIndexCnt = 0;
            while (worker != NULL) {
                if (worker->data == nWorkerSocket) {
                    if (remove_from_list(workers, nIndexCnt)) {
                        print_hash_table(nClientWorkerSocketTable);
                    }
                    break; // Prekida se petlja nakon brisanja
                }
                nIndexCnt++;
                worker = worker->next;
            }
        }
    }
    else {
        printf("\nCANT HAPPEN NOW\n");
    }
}


void ProcessNewMessage(int nClientSocket) {
    printf("\nProcessing message from client: %d\n", nClientSocket);
    char buffer[257] = { 0 }; // Dodat jedan bajt za null terminator

    // Provera stanja socket-a pre čitanja
    int nRet = recv(nClientSocket, buffer, 256, 0);
    if (nRet <= 0) {
        // Zatvaranje socket-a ako je prekinuta veza ili došlo do greške
        printf("\nSomething bad happened. Closing Client Socket %d\n", nClientSocket);
        closesocket(nClientSocket);

        LIST* clients = get_table_item(nClientWorkerSocketTable, "clients");
        if (clients != NULL && clients->count > 0) {
            LIST_ITEM* client = clients->head;
            int nIndexCnt = 0;
            while (client != NULL) {
                if (client->data == nClientSocket) {
                    if (remove_from_list(clients, nIndexCnt)) {
                        print_hash_table(nClientWorkerSocketTable);
                    }
                    break; // Prekida se petlja nakon brisanja
                }
                nIndexCnt++;
                client = client->next;
            }
        }
    }
    else {
        printf("\n[Client]: %s\n", buffer);

        // Slanje povratne poruke klijentu
        send(nClientSocket, "Got message!", 13, 0);
        printf("\n******************\n");

        char helper[50];
        snprintf(helper, sizeof(helper), "client-%d", nClientSocket);
        const char* clientName = helper;
        printf("\n%s\n", clientName);

        // Dodavanje klijenta u tabelu poruka ako ne postoji
        if (!get_table_item_msg(nClientMSGTable, clientName)) {
            add_list_table_msg(nClientMSGTable, clientName);
        }

        // Dodavanje poruke u tabelu poruka
        add_table_item_msg(nClientMSGTable, clientName, buffer);

        // Štampanje trenutnog stanja tabele poruka
        print_hash_table_msg(nClientMSGTable);

        // Dodavanje poruke u red za procesiranje
        enqueue(nClientMsgsQueue, create_queue_element(clientName, buffer));
    }
}
