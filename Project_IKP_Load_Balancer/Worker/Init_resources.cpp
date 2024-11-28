#include "init_resources.h"

// Function to initialize all required resources
void initialize_resources( HASH_TABLE_MSG** msgTable, QUEUE** msgQueue) {

    // Initialize message hash table
    *msgTable = init_hash_table_msg();

    // Initialize message queue
    *msgQueue = init_queue(QUEUESIZE);

    printf("Resources initialized successfully.\n");
}

void free_resources(HASH_TABLE_MSG** msgTable, QUEUE** msgQueue) {

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