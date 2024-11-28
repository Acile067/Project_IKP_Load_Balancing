#include "networking_utils.h"

extern HASH_TABLE* nClientWorkerSocketTable;
extern QUEUE* nClientMsgsQueue;
extern HASH_TABLE_MSG* nClientMSGTable;
extern WorkerArray g_WorkerArray;
extern CRITICAL_SECTION g_workerArrayCriticalSection;

// Inicijalizacija kritične sekcije
void initialize_worker_array_critical_section() {
    InitializeCriticalSection(&g_workerArrayCriticalSection);
}

// Čišćenje kritične sekcije
void cleanup_worker_array_critical_section() {
    DeleteCriticalSection(&g_workerArrayCriticalSection);
}
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

            if (send_hash_table(clientSocket, nClientMSGTable) == 0) {                  //From: networking_utils.h
                printf("Hash table sent successfully to worker.\n");
            }

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
    SOCKET workerSocket = (SOCKET)nWorkerSocket;
    // Provera stanja socket-a pre čitanja
    int nRet = recv(nWorkerSocket, buffer, 256, 0);
    if (nRet <= 0) {
        // Zatvaranje socket-a ako je prekinuta veza ili doslo do greske
        printf("\nSomething bad happened. Closing Worker Socket %d\n", nWorkerSocket);
        closesocket(nWorkerSocket);
        remove_worker_from_array(&g_WorkerArray, workerSocket);
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
        int result = recv_and_handle_worker_message(workerSocket, buffer, nRet, &g_WorkerArray);
        if (result < 0) {
            printf("Worker socket %d disconnected or failed to process message.\n", workerSocket);
        }
        else {
            printf("Message from worker socket %d processed successfully.\n", workerSocket);
        }
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

int serialize_hash_table(HASH_TABLE_MSG* table, char* buffer, size_t size) {
    if (table == NULL || buffer == NULL || size == 0) {
        printf("serialize_hash_table() failed: invalid parameters\n");
        return -1;
    }

    if (table->count == 0) {
        snprintf(buffer, size, "empty"); // Popunite buffer sa "empty"
        return 0;
    }
    convert_to_string(table, buffer, size);                                     //From ../Common/Hashtable.h
    return 0; // Uspelo
}

int send_hash_table(SOCKET socket, HASH_TABLE_MSG* table) {
    char buffer[4096]; // Maksimalna veličina serijalizovanih podataka
    if (serialize_hash_table(table, buffer, sizeof(buffer)) != 0) {
        printf("send_hash_table() failed: serialization failed\n");
        return -1;
    }

    // Slanje podataka preko mreže
    int result = send(socket, buffer, strlen(buffer), 0);
    if (result == SOCKET_ERROR) {
        printf("send() failed: %d\n", WSAGetLastError());
        return -1;
    }
    return 0; // Uspelo
}

int handle_worker_message(SOCKET workerSocket, char* message, WorkerArray* workers) {
    if (strncmp(message, "PORT:", 5) == 0) {
        uint16_t port = (uint16_t)atoi(message + 5);
        if (add_worker_to_array(workers, workerSocket, port) != 0) {
            printf("Failed to add worker with port %u to array.\n", port);
            return -1;
        }
        printf("Worker with port %u added successfully.\n", port);
    }
    else {
        printf("Unknown message received: %s\n", message);
    }
    return 0;
}

int add_worker_to_array(WorkerArray* array, SOCKET socket, uint16_t port) {
    EnterCriticalSection(&g_workerArrayCriticalSection);

    if (array->count >= MAX_WORKERS) {
        printf("Worker array is full.\n");
        // Otključavanje kritične sekcije pre nego što se izađe
        LeaveCriticalSection(&g_workerArrayCriticalSection);
        return -1;
    }

    array->workers[array->count].socket = socket;
    array->workers[array->count].port = port;
    array->count++;

    // Otključavanje kritične sekcije
    LeaveCriticalSection(&g_workerArrayCriticalSection);

    return 0;
}

int recv_and_handle_worker_message(SOCKET workerSocket, char* buffer, int bufferLength, WorkerArray* workers) {
    if (buffer == NULL || bufferLength <= 0) {
        printf("Invalid buffer or buffer length.\n");
        return -1;
    }

    // Dodaj nul-terminator za sigurnost
    buffer[bufferLength] = '\0';

    printf("Message received from worker: %s\n", buffer);

    // Obrada poruke
    if (handle_worker_message(workerSocket, buffer, workers) != 0) {
        printf("Failed to handle worker message: %s\n", buffer);
        return -1;
    }

    return 0; // Uspeh
}


int remove_worker_from_array(WorkerArray* workers, SOCKET socket) {
    EnterCriticalSection(&g_workerArrayCriticalSection);

    for (int i = 0; i < workers->count; i++) {
        if (workers->workers[i].socket == socket) {
            // Pomeri radnike u nizu ulevo
            for (int j = i; j < workers->count - 1; j++) {
                workers->workers[j] = workers->workers[j + 1];
            }
            workers->count--;
            printf("Worker with socket %d removed from array.\n");
            // Otključavanje kritične sekcije pre nego što se izađe
            LeaveCriticalSection(&g_workerArrayCriticalSection);
            return 0;
        }
    }

    // Otključavanje kritične sekcije pre nego što se izađe
    LeaveCriticalSection(&g_workerArrayCriticalSection);
    printf("Worker with socket %d not found in array.\n");
    return -1;
}