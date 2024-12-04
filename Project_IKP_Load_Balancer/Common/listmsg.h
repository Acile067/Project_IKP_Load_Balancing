#pragma once

#include <windows.h>
#include <iostream>

using namespace std;

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


#pragma region headers for the _MSG structure

LIST_MSG* init_list_msg();
void add_list_front_msg(LIST_MSG* list, LIST_ITEM_MSG data);
LIST_ITEM_MSG* get_list_item_msg(LIST_MSG* list, int index);
bool remove_from_list_msg(LIST_MSG* list, int index);
bool clear_list_msg(LIST_MSG* list);
bool free_list_msg(LIST_MSG** list);
void print_list_msg(LIST_MSG* list);

#pragma endregion