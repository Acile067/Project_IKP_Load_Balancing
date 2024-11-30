#include "networking_utils.h"

extern QUEUE* nClientMsgsQueue;
extern HASH_TABLE_MSG* nClientMSGTable;
extern PORT_QUEUE* queueWithClientNameMsgPorts;

int receive_hash_table(SOCKET socket, char* buffer, size_t size) {
    if (buffer == NULL || size == 0) {
        printf("receive_hash_table() failed: invalid buffer\n");
        return -1;
    }

    memset(buffer, 0, size); // Resetuj buffer
    int result = recv(socket, buffer, size - 1, 0); // Ostaviti mesta za '\0'
    if (result <= 0) {
        printf("recv() failed: %d\n", WSAGetLastError());
        return -1;
    }

    buffer[result] = '\0'; // Dodaj terminirajući karakter
    return 0; // Uspelo
}

int split_string(const char* str, char delimiter, char output[MAX_TOKENS][MAX_TOKEN_LEN]) {
    int count = 0;
    char* token;
    char* str_copy = _strdup(str);  // Napravite kopiju stringa
    char* context = nullptr;  // Kontekst za strtok_s

    token = strtok_s(str_copy, &delimiter, &context);  // Koristimo sigurniju verziju strtok

    while (token != NULL && count < MAX_TOKENS) {
        strncpy_s(output[count], token, MAX_TOKEN_LEN - 1);  // Dodajte -1 da biste ostavili prostor za '\0'
        output[count][MAX_TOKEN_LEN - 1] = '\0';  // Osigurajte da string bude nul-terminiran
        token = strtok_s(NULL, &delimiter, &context);  // Za sledeći token
        count++;
    }

    free(str_copy);  // Oslobađanje memorije
    return count;
}

int ParseFromStringToHashTable(char* data)
{
    printf("Data to parse: %s\n", data);

    // Podela podataka po ';' da bi se dobili klijenti
    char clients[MAX_TOKENS][MAX_TOKEN_LEN];
    int num_clients = split_string(data, ';', clients);

    for (int i = 0; i < num_clients; i++) {
        if (strlen(clients[i]) == 0) continue;  // Preskočite prazne stringove

        // Definišite kontekst za strtok_s
        char* context = nullptr;

        // Podelite klijenta po ":" da biste dobili ime klijenta i poruke
        char* client_name = strtok_s(clients[i], ":", &context);
        char* messages_str = strtok_s(nullptr, ":", &context);

        // Podela poruka po ','
        char messages[MAX_TOKENS][MAX_TOKEN_LEN];
        int num_messages = split_string(messages_str, ',', messages);

        // Dodavanje klijenta u tabelu
        if (add_list_table_msg(nClientMSGTable, client_name)) {}

        // Dodavanje poruka za tog klijenta
        for (int j = num_messages - 1; j >= 0; j--) {
            char* msg = messages[j]; // Poruka je već string, nije potrebno konvertovati u broj
            if (add_table_item_msg(nClientMSGTable, client_name, msg)) {}
        }
    }
    return 0;
}

int receive_and_deserialize(SOCKET socket) {
    char buffer[4096]; // Maksimalna veličina serijalizovanih podataka

    if (receive_hash_table(socket, buffer, sizeof(buffer)) != 0) {
        printf("receive_and_deserialize() failed: receiving failed\n");
        return -1;
    }

    //hash tabela je prazna na strani load balancera
    if (strcmp(buffer, "empty") == 0) {
        printf("Hash table is (empty).\n");
        return 0; // Nema podataka za parsiranje
    }

    if (ParseFromStringToHashTable(buffer) != 0) {
        printf("receive_and_deserialize() failed: deserialization failed\n");
        return -1;
    }

    return 0; // Uspelo
}

int send_worker_port(SOCKET socket, uint16_t port) {
    if (socket == INVALID_SOCKET) {
        printf("send_worker_port() failed: invalid socket\n");
        return -1;
    }

    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "PORT:%u", port);

    int result = send(socket, buffer, (int)strlen(buffer), 0);
    if (result == SOCKET_ERROR) {
        printf("send_worker_port() failed: %d\n", WSAGetLastError());
        return -1;
    }

    printf("Worker port (%u) sent successfully to load balancer.\n", port);
    return 0;
}

int deserialize_combined_data_structure(char* buffer, int bufferSize, CombinedDataStructure* data) {
    if (!buffer || bufferSize <= 0 || !data) return -1;

    int offset = 0;

    // Čitanje clientName (dužina + sadržaj)
    int nameLength = 0;
    if (offset + sizeof(int) > bufferSize) return -1;
    memcpy(&nameLength, buffer + offset, sizeof(int));
    offset += sizeof(int);

    if (nameLength > 0) {
        data->clientName = (char*)malloc(nameLength);
        if (!data->clientName) return -1;
        if (offset + nameLength > bufferSize) return -1;
        memcpy(data->clientName, buffer + offset, nameLength);
        offset += nameLength;
    }
    else {
        data->clientName = NULL;
    }

    // Čitanje data (dužina + sadržaj)
    int dataLength = 0;
    if (offset + sizeof(int) > bufferSize) return -1;
    memcpy(&dataLength, buffer + offset, sizeof(int));
    offset += sizeof(int);

    if (dataLength > 0) {
        data->data = (char*)malloc(dataLength);
        if (!data->data) {
            free(data->clientName);
            return -1;
        }
        if (offset + dataLength > bufferSize) return -1;
        memcpy(data->data, buffer + offset, dataLength);
        offset += dataLength;
    }
    else {
        data->data = NULL;
    }

    // Čitanje WorkerArray
    if (offset + sizeof(WorkerArray) > bufferSize) {
        free(data->clientName);
        free(data->data);
        return -1;
    }
    memcpy(&data->workerArray, buffer + offset, sizeof(WorkerArray));
    offset += sizeof(WorkerArray);

    return 0; // Uspešno deserijalizovano
}

void receive_combined_data(SOCKET serverSocket) {
    char buffer[1024];
    int bytesReceived = recv(serverSocket, buffer, sizeof(buffer), 0);

    if (bytesReceived <= 0) {
        printf("Failed to receive data or connection closed.\n");
        return;
    }

    // Kreiranje CombinedDataStructure za preuzete podatke
    CombinedDataStructure receivedData;
    memset(&receivedData, 0, sizeof(receivedData));

    if (deserialize_combined_data_structure(buffer, bytesReceived, &receivedData) == 0) {
        printf("Client Name: %s\n", receivedData.clientName ? receivedData.clientName : "NULL");
        printf("Client Data: %s\n", receivedData.data ? receivedData.data : "NULL");

        printf("Worker Array:\n");
        uint16_t* ports = (uint16_t*)malloc(receivedData.workerArray.count * sizeof(uint16_t));
        if (!ports) {
            printf("Failed to allocate memory for ports.\n");
            return;
        }

        for (int i = 0; i < receivedData.workerArray.count; i++) {
            printf("  Worker %d - Socket: %d, Port: %u\n",
                i,
                receivedData.workerArray.workers[i].socket,
                receivedData.workerArray.workers[i].port);

            ports[i] = receivedData.workerArray.workers[i].port;
        }

        //Dodajemo u tabelu
        if (!get_table_item_msg(nClientMSGTable, receivedData.clientName)) {
            add_list_table_msg(nClientMSGTable, receivedData.clientName);
        }

        add_table_item_msg(nClientMSGTable, receivedData.clientName, receivedData.data);

        print_hash_table_msg(nClientMSGTable);

                                                                                // "client-123", "msg1", {5555, 6666}, 2
        PORT_QUEUEELEMENT* element = create_port_queue_element(receivedData.clientName, receivedData.data, ports, receivedData.workerArray.count);

        enqueue_port(queueWithClientNameMsgPorts, element);

        print_port_queue(queueWithClientNameMsgPorts);

        free(ports);
    }
    else {
        printf("Failed to deserialize received data.\n");
    }

    // Oslobađanje memorije
    free(receivedData.clientName);
    free(receivedData.data);
}


int serialize_message(const ClientMessage* message, char** buffer, int* size) {
    int clientNameLen = strlen(message->clientName) + 1; // +1 za '\0'
    int dataLen = strlen(message->data) + 1;

    *size = sizeof(int) * 2 + clientNameLen + dataLen;
    *buffer = (char*)malloc(*size);
    if (*buffer == nullptr) {
        return -1; // Alokacija memorije nije uspela
    }

    char* ptr = *buffer;
    memcpy(ptr, &clientNameLen, sizeof(int));
    ptr += sizeof(int);
    memcpy(ptr, message->clientName, clientNameLen);
    ptr += clientNameLen;
    memcpy(ptr, &dataLen, sizeof(int));
    ptr += sizeof(int);
    memcpy(ptr, message->data, dataLen);

    return 0; // Uspelo
}

int deserialize_message(const char* buffer, ClientMessage* message) {
    const char* ptr = buffer;
    int clientNameLen, dataLen;

    memcpy(&clientNameLen, ptr, sizeof(int));
    ptr += sizeof(int);
    message->clientName = (char*)malloc(clientNameLen);
    if (message->clientName == nullptr) return -1;
    memcpy(message->clientName, ptr, clientNameLen);
    ptr += clientNameLen;

    memcpy(&dataLen, ptr, sizeof(int));
    ptr += sizeof(int);
    message->data = (char*)malloc(dataLen);
    if (message->data == nullptr) {
        free(message->clientName);
        return -1;
    }
    memcpy(message->data, ptr, dataLen);

    return 0; // Uspelo
}

