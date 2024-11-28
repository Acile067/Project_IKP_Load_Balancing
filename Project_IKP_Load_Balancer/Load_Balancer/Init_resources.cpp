#include "init_resources.h"

// Function to initialize all required resources
void initialize_resources(HASH_TABLE** socketTable, HASH_TABLE_MSG** msgTable, QUEUE** msgQueue) {
    // Initialize socket hash table
    *socketTable = init_hash_table();
    if (*socketTable) {
        add_list_table(*socketTable, "clients");
        add_list_table(*socketTable, "workers");
    }

    // Initialize message hash table
    *msgTable = init_hash_table_msg();

    // Initialize message queue
    *msgQueue = init_queue(QUEUESIZE);

    printf("Resources initialized successfully.\n");
}

void free_resources(HASH_TABLE** socketTable, HASH_TABLE_MSG** msgTable, QUEUE** msgQueue) {
    // Free socket hash table
    if (socketTable != NULL && *socketTable != NULL) {
        if (!free_hash_table(socketTable)) {
            cout << "Failed to free socket table." << endl;
        }
    }

    // Free message hash table
    if (msgTable != NULL && *msgTable != NULL) {
        if (!free_hash_table_msg(msgTable)) {
            cout << "Failed to free message table." << endl;
        }
    }

    // Free message queue
    if (msgQueue != NULL && *msgQueue != NULL) {
        delete_queue(*msgQueue);
        *msgQueue = NULL;
    }

    printf("Resources freed successfully.\n");
}