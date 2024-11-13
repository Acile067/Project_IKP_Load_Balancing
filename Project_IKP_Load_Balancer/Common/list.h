#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock.h>  // Koriš?enje Winsock1 (umesto Winsock2.h)

#define MAX_LIST_SIZE 100

typedef struct _LIST_ITEM
{
    SOCKET data;  // SOCKET tip
    struct _LIST_ITEM* next;
} LIST_ITEM;

typedef struct _LIST
{
    LIST_ITEM* head;
    LIST_ITEM* tail;
    CRITICAL_SECTION cs;
    int count;
} LIST;

typedef struct _LIST_ITEM_MSG
{
    const char* data;  // char* tip
    struct _LIST_ITEM_MSG* next;
} LIST_ITEM_MSG;

typedef struct _LIST_MSG
{
    _LIST_ITEM_MSG* head;
    _LIST_ITEM_MSG* tail;
    CRITICAL_SECTION cs;
    int count;
} LIST_MSG;

/// <summary>
/// Initialize list
/// </summary>
/// <returns>Pointer to initialized list</returns>
LIST* init_list();


/// <summary>
/// Add item to front of the list
/// </summary>
/// <param name="list"> - source list</param>
/// <param name="data"> - list item to be added</param>
void add_list_front(LIST* list, LIST_ITEM data);


/// <summary>
/// Add item to back of the list
/// </summary>
/// <param name="list"> - source list</param>
/// <param name="data"> - list item to be added</param>
void add_list_back(LIST* list, LIST_ITEM data);

/// <summary>
/// Get list item at index
/// </summary>
/// <param name="list"> - source list</param>
/// <param name="index"> - index of the list item</param>
/// <returns>Returns list item at given index if exists, otherwise NULL </returns>
LIST_ITEM* get_list_item(LIST* list, int index);

/// <summary>
/// Remove list item at index
/// </summary>
/// <param name="list"> - source list</param>
/// <param name="index"> - index of the list item</param>
/// <returns>True if successful, otherwise false</returns>
bool remove_from_list(LIST* list, int index);

/// <summary>
/// Clear list
/// </summary>
/// <param name="list"></param>
/// <returns>True if successful, otherwise false</returns>
bool clear_list(LIST* list);

/// <summary>
/// Destroy list
/// </summary>
/// <param name="list"></param>
/// <returns>True if successful, otherwise false</returns>
bool free_list(LIST** list);

/// <summary>
/// Print list
/// </summary>
/// <param name="list"> - source list</param>
void print_list(LIST* list);


#pragma region headers for the _MSG structure

LIST_MSG* init_list_msg();
void add_list_front_msg(LIST_MSG* list, LIST_ITEM_MSG data);
LIST_ITEM_MSG* get_list_item_msg(LIST_MSG* list, int index);
bool remove_from_list_msg(LIST_MSG* list, int index);
bool clear_list_msg(LIST_MSG* list);
bool free_list_msg(LIST_MSG** list);
void print_list_msg(LIST_MSG* list);

#pragma endregion
