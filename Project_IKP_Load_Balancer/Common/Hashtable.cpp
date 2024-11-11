#pragma once
#include "hashtable.h"
#include <winsock.h> 
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <windows.h>  // Za CriticalSection

using namespace std;

// Inicijalizacija djb2hash tabele
HASH_TABLE* init_hash_table()
{
    HASH_TABLE* table = (HASH_TABLE*)malloc(sizeof(HASH_TABLE));
    if (table == NULL)
    {
        cout << "init_hash_table() failed: out of memory" << endl;
        return NULL;
    }
    table->count = 0;

    table->items = (HASH_ITEM*)malloc(sizeof(HASH_ITEM) * TABLE_SIZE);
    if (table->items == NULL)
    {
        cout << "init_hash_table() failed: out of memory" << endl;
        return NULL;
    }

    for (int i = 0; i < TABLE_SIZE; i++)
    {
        table->items[i].list = NULL;
        table->items[i].key = NULL;
    }

    InitializeCriticalSection(&table->cs);

    return table;
}

// Hash funkcija koristi djb2 algoritam
unsigned int djb2hash(const char* key)
{
    if (key == NULL)
    {
        cout << "djb2hash() failed: key is NULL" << endl;
        return -1;
    }
    if (strlen(key) > MAX_KEY_LEN)
    {
        cout << "djb2hash() failed: key is too long" << endl;
        return -1;
    }

    unsigned int hash_value = 0;

    while (*key != '\0') {
        hash_value = (hash_value << 5) + *key++;
    }

    return hash_value % TABLE_SIZE;
}

// Dodavanje liste u djb2hash tabelu
bool add_list_table(HASH_TABLE* table, const char* key)
{
    if (table == NULL)
    {
        cout << "add_list_table() failed: table is NULL" << endl;
        return false;
    }

    if (key == NULL)
    {
        cout << "add_list_table() failed: key is NULL" << endl;
        return false;
    }

    if (strlen(key) > MAX_KEY_LEN)
    {
        cout << "add_list_table() failed: key is too long" << endl;
        return false;
    }

    EnterCriticalSection(&table->cs);

    int index = djb2hash(key);
    if (index == -1)
    {
        cout << "add_list_table() failed: djb2hash() failed" << endl;
        LeaveCriticalSection(&table->cs);
        return false;
    }

    table->items[index].list = init_list();
    if (table->items[index].list == NULL)
    {
        cout << "add_list_table() failed: init_list() failed" << endl;
        LeaveCriticalSection(&table->cs);
        return false;
    }

    table->items[index].key = (char*)malloc(strlen(key) + 1);
    if (table->items[index].key == NULL) {
        cout << "add_list_table() failed: out of memory" << endl;
        LeaveCriticalSection(&table->cs);
        return false;
    }

    strcpy_s(table->items[index].key, strlen(key) + 1, key);
    table->count++;
    LeaveCriticalSection(&table->cs);
    return true;
}

// Dodavanje stavke u tabelu
bool add_table_item(HASH_TABLE* table, const char* key, SOCKET sock)
{
    if (table == NULL)
    {
        cout << "add_table_item() failed: table is NULL" << endl;
        return false;
    }

    if (key == NULL)
    {
        cout << "add_table_item() failed: key is NULL" << endl;
        return false;
    }

    if (strlen(key) > MAX_KEY_LEN)
    {
        cout << "add_table_item() failed: key is too long" << endl;
        return false;
    }

    EnterCriticalSection(&table->cs);

    int index = djb2hash(key);
    if (index == -1)
    {
        cout << "add_table_item() failed: djb2hash() failed" << endl;
        LeaveCriticalSection(&table->cs);
        return false;
    }
    HASH_ITEM* item = &table->items[index];
    if (item->list == NULL) {
        cout << "add_table_item() failed: list for key is NULL" << endl;
    }

    LIST_ITEM newItem = { sock, NULL };
    add_list_front(item->list, newItem);

    LeaveCriticalSection(&table->cs);
    return true;
}

//// Dodavanje msg u tabelu
//bool add_table_item_msg(HASH_TABLE* table, const char* key, char* msg)
//{
//    if (table == NULL)
//    {
//        cout << "add_table_item() failed: table is NULL" << endl;
//        return false;
//    }
//
//    if (key == NULL)
//    {
//        cout << "add_table_item() failed: key is NULL" << endl;
//        return false;
//    }
//
//    if (strlen(key) > MAX_KEY_LEN)
//    {
//        cout << "add_table_item() failed: key is too long" << endl;
//        return false;
//    }
//
//    EnterCriticalSection(&table->cs);
//
//    int index = djb2hash(key);
//    if (index == -1)
//    {
//        cout << "add_table_item() failed: djb2hash() failed" << endl;
//        LeaveCriticalSection(&table->cs);
//        return false;
//    }
//    HASH_ITEM* item = &table->items[index];
//    if (item->list == NULL) {
//        cout << "add_table_item() failed: list for key is NULL" << endl;
//    }
//
//    LIST_ITEM_MSG newItem = { msg, NULL };
//    add_list_front(item->list, newItem);
//
//    LeaveCriticalSection(&table->cs);
//    return true;
//}

// Dohvatanje stavke iz djb2hash tabele
LIST* get_table_item(HASH_TABLE* table, const char* key)
{
    if (table == NULL)
    {
        cout << "get_table_item() failed: table is NULL" << endl;
        return NULL;
    }

    if (key == NULL)
    {
        cout << "get_table_item() failed: key is NULL" << endl;
        return NULL;
    }

    if (strlen(key) > MAX_KEY_LEN)
    {
        cout << "get_table_item() failed: key is too long" << endl;
        return NULL;
    }

    EnterCriticalSection(&table->cs);

    int index = djb2hash(key);
    if (index == -1)
    {
        cout << "get_table_item() failed: djb2hash() failed" << endl;
        LeaveCriticalSection(&table->cs);
        return NULL;
    }

    HASH_ITEM item = table->items[index];
    LeaveCriticalSection(&table->cs);
    return item.list;
}

// Provera da li ključ postoji u djb2hash tabeli
bool has_key(HASH_TABLE* table, const char* key)
{
    if (table == NULL)
    {
        cout << "has_key() failed: table is NULL" << endl;
        return false;
    }

    if (key == NULL)
    {
        cout << "has_key() failed: key is NULL" << endl;
        return false;
    }

    if (strlen(key) > MAX_KEY_LEN)
    {
        cout << "has_key() failed: key is too long" << endl;
        return false;
    }

    EnterCriticalSection(&table->cs);

    int index = djb2hash(key);
    if (index == -1)
    {
        cout << "has_key() failed: djb2hash() failed" << endl;
        LeaveCriticalSection(&table->cs);
        return false;
    }

    if (table->items[index].list == NULL)
    {
        LeaveCriticalSection(&table->cs);
        return false;
    }

    LeaveCriticalSection(&table->cs);
    return true;
}

// Uklanjanje stavke iz djb2hash tabele
bool remove_table_item(HASH_TABLE* table, const char* key)
{
    if (table == NULL)
    {
        cout << "remove_table_item() failed: table is NULL" << endl;
        return false;
    }

    if (key == NULL)
    {
        cout << "remove_table_item() failed: key is NULL" << endl;
        return false;
    }

    if (strlen(key) > MAX_KEY_LEN)
    {
        cout << "remove_table_item() failed: key is too long" << endl;
        return false;
    }

    EnterCriticalSection(&table->cs);

    int index = djb2hash(key);
    if (index == -1)
    {
        cout << "remove_table_item() failed: djb2hash() failed" << endl;
        LeaveCriticalSection(&table->cs);
        return false;
    }

    if (strcmp(table->items[index].key, key) != 0)
    {
        cout << "remove_table_item() failed: key not found" << endl;
        LeaveCriticalSection(&table->cs);
        return false;
    }

    if (!clear_list(table->items[index].list))
    {
        cout << "remove_table_item() failed: clear_list() failed" << endl;
        LeaveCriticalSection(&table->cs);
        return false;
    }

    table->count--;

    LeaveCriticalSection(&table->cs);
    return true;
}

// Oslobađanje djb2hash tabele
bool free_hash_table(HASH_TABLE** table)
{
    if (table == NULL || *table == NULL)
    {
        cout << "free_hash_table() failed: table is NULL" << endl;
        return true;
    }

    EnterCriticalSection(&(*table)->cs);
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        if ((*table)->items[i].list != NULL)
        {
            if (!free_list(&(*table)->items[i].list))
            {
                cout << "free_hash_table() failed: could not free list at index " << i << endl;
                LeaveCriticalSection(&(*table)->cs);
                return false;
            }
        }

        if ((*table)->items[i].key != NULL)
        {
            free((*table)->items[i].key);
            (*table)->items[i].key = NULL;
        }
    }
    LeaveCriticalSection(&(*table)->cs);

    DeleteCriticalSection(&(*table)->cs);
    free((*table)->items);
    (*table)->items = NULL;

    free(*table);
    *table = NULL;

    return true;
}

// Ispisivanje sadržaja djb2hash tabele
void print_hash_table(HASH_TABLE* table)
{
    if (table == NULL)
    {
        cout << "print_hashtable(): table is NULL" << endl;
        return;
    }

    cout << "======== djb2hash Table ========" << endl;
    cout << "Count: " << table->count << endl << endl;
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        if (table->items[i].key != NULL) {
            cout << "Index: " << i << endl;
            cout << "Key: " << table->items[i].key << endl;
            cout << "List:" << endl;
            print_list(table->items[i].list);
            cout << endl;
        }
    }
    cout << "============================" << endl;
}
