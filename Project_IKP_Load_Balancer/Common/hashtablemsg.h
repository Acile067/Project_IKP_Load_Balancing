#pragma once

#include "listmsg.h"

#define TABLE_SIZE 1000
#define MAX_KEY_LEN 128 + 1

typedef struct _HASH_ITEM_MSG
{
    char* key;
    LIST_MSG* list;
} HASH_ITEM_MSG;

typedef struct _HASH_TABLE_MSG
{
    CRITICAL_SECTION cs;
    _HASH_ITEM_MSG* items;
    int count;
} HASH_TABLE_MSG;

#pragma region headers for the _MSG structure

HASH_TABLE_MSG* init_hash_table_msg();
unsigned int djb2hash_msg(const char* key);
bool add_table_item_msg(HASH_TABLE_MSG* table, const char* key, const char* data);
bool add_list_table_msg(HASH_TABLE_MSG* table, const char* key);
LIST_MSG* get_table_item_msg(HASH_TABLE_MSG* table, const char* key);
bool has_key_msg(HASH_TABLE_MSG* table, const char* key);
bool remove_table_item_msg(HASH_TABLE_MSG* table, const char* key);
bool free_hash_table_msg(HASH_TABLE_MSG** table);
void print_hash_table_msg(HASH_TABLE_MSG* table);
void convert_to_string(HASH_TABLE_MSG* table, char* ret, size_t size);

#pragma endregion
