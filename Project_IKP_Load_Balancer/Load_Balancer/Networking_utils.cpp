#include "networking_utils.h"

extern HASH_TABLE* nClientWorkerSocketTable;
// Function to process a new client request
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

    // Close the client socket after handling
    closesocket(clientSocket);
}
