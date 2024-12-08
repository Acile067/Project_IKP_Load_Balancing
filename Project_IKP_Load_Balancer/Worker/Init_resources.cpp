#include "init_resources.h"

// Function to initialize all required resources
void initialize_resources( HASH_TABLE_MSG** msgTable, QUEUE** msgQueue, PORT_QUEUE** portQueue, THREAD_QUEUE** threadPoolQueue) {

    // Initialize message hash table
    *msgTable = init_hash_table_msg();

    // Initialize message queue
    *msgQueue = init_queue(QUEUESIZE);

    // Inicijalizacija reda sa portovima
    *portQueue = init_port_queue(15); // Kapacitet može biti dinamičan

    *threadPoolQueue = init_thread_queue(20);

    printf("Resources initialized successfully.\n");
}

void free_resources(HASH_TABLE_MSG** msgTable, QUEUE** msgQueue, PORT_QUEUE** portQueue, THREAD_QUEUE** threadPoolQueue) {

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

    // Oslobađanje reda sa portovima
    if (portQueue != NULL && *portQueue != NULL) {
        delete_port_queue(*portQueue);
        *portQueue = NULL;
    }

    if (threadPoolQueue != NULL && *threadPoolQueue != NULL) {
        delete_thread_queue(*threadPoolQueue);
        *threadPoolQueue = NULL;
    }

    printf("Resources freed successfully.\n");
}