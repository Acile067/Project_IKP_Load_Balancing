#include "networking_utils.h"

extern QUEUE* nClientMsgsQueue;
extern HASH_TABLE_MSG* nClientMSGTable;

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