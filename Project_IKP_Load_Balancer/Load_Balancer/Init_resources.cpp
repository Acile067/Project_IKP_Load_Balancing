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
