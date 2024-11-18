#pragma once
#include "string.h"
#include "list.h"
#include "common.h"
#include <winsock.h>

#define TABLE_SIZE 1000
#define MAX_KEY_LEN 128 + 1

#pragma region Hashtable structure
typedef struct _HASH_ITEM
{
    char* key;
    LIST* list;
} HASH_ITEM;

typedef struct _HASH_TABLE
{
    CRITICAL_SECTION cs;
    HASH_ITEM* items;
    int count;
} HASH_TABLE;
#pragma endregion

#pragma region HashtabelMSG structure
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
#pragma endregion

#pragma region HashtabelINT structure
typedef struct _HASH_ITEM_INT
{
    char* key;
    int value;
} HASH_ITEM_INT;

typedef struct _HASH_TABLE_INT
{
    CRITICAL_SECTION cs;
    HASH_ITEM_INT* items;
    int count;
} HASH_TABLE_INT;
#pragma endregion

#pragma region Headers for the Hastable structure

/// <summary>
/// Initialize hash table
/// </summary>
/// <returns>Handle if initialization is successful, otherwise NULL</returns>
HASH_TABLE* init_hash_table();

/// <summary>
/// Hash function using djb2 algorithm
/// </summary>
/// <param name="key"> - key to hash</param>
/// <returns>Hashed key</returns>
unsigned int djb2hash(const char* key);

/// <summary>
/// Add item to hash table
/// </summary>
/// <param name="table"> - source table</param>
/// <param name="key"> - table item key</param>
/// <param name="sock"> - item to add</param>
/// <returns>True if addition is successful, otherwise false</returns>
bool add_table_item(HASH_TABLE* table, const char* key, SOCKET sock);


/// <summary>
/// Add list to hash table
/// </summary>
/// <param name="table"> - source table</param>
/// <param name="key"> - table item key</param>
/// <returns>True if addition is successful, otherwise false</returns>
bool add_list_table(HASH_TABLE* table, const char* key);

/// <summary>
/// Get table item
/// </summary>
/// <param name="table"> - source table</param>
/// <param name="key"> - item key</param>
/// <returns>Table item if exists, otherwise NULL</returns>
LIST* get_table_item(HASH_TABLE* table, const char* key);

/// <summary>
/// Check if table has item with specified key
/// </summary>
/// <param name="table"> - source table</param>
/// <param name="key"> - item key</param>
/// <returns>True if table has item with specified key, otherwise false</returns>
bool has_key(HASH_TABLE* table, const char* key);

/// <summary>
/// Remove item from table
/// </summary>
/// <param name="table"> - source table</param>
/// <param name="key"> - item key</param>
/// <returns>True if table item successfully removed, otherwise false</returns>
bool remove_table_item(HASH_TABLE* table, const char* key);

/// <summary>
/// Free hash table
/// </summary>
/// <param name="table"> - table to free</param>
/// <returns>True if table successfully freed, otherwise false</returns>
bool free_hash_table(HASH_TABLE** table);

/// <summary>
/// Print hash table
/// </summary>
/// <param name="table"> - table to print</param>
void print_hash_table(HASH_TABLE* table);

#pragma endregion

#pragma region headers for the MESSAGES structure

HASH_TABLE_MSG* init_hash_table_msg();
bool add_table_item_msg(HASH_TABLE_MSG* table, const char* key, const char* data);
bool add_list_table_msg(HASH_TABLE_MSG* table, const char* key);
LIST_MSG* get_table_item_msg(HASH_TABLE_MSG* table, const char* key);
bool has_key_msg(HASH_TABLE_MSG* table, const char* key);
bool remove_table_item_msg(HASH_TABLE_MSG* table, const char* key);
bool free_hash_table_msg(HASH_TABLE_MSG** table);
void print_hash_table_msg(HASH_TABLE_MSG* table);
void convert_to_string(HASH_TABLE_MSG* table, char* ret, size_t size);

#pragma endregion

#pragma region headers for the INTEGER structure

HASH_TABLE_INT* init_hash_table_int();
bool add_table_item_int(HASH_TABLE_INT* table, const char* key, int value);
HASH_ITEM_INT* get_table_item_int(HASH_TABLE_INT* table, const char* key);
bool has_key_int(HASH_TABLE_INT* table, const char* key);
bool remove_table_item_int(HASH_TABLE_INT* table, const char* key);
bool free_hash_table_int(HASH_TABLE_INT** table);
void print_hash_table_int(HASH_TABLE_INT* table);

#pragma endregion
