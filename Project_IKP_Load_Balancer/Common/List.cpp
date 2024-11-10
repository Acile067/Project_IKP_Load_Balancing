#pragma once
#include "list.h"
#include <winsock.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <windows.h>  // Za CriticalSection

using namespace std;

// Inicijalizacija liste
LIST* init_list()
{
    LIST* list = (LIST*)malloc(sizeof(LIST));
    if (list == NULL)
    {
        cout << "init_list() failed: out of memory" << endl;
        return NULL;
    }

    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
    InitializeCriticalSection(&list->cs);

    return list;
}

// Dodavanje elementa na početak liste
void add_list_front(LIST* list, LIST_ITEM data)
{
    if (list == NULL)
    {
        cout << "add_list_front() failed: list is NULL" << endl;
        return;
    }

    EnterCriticalSection(&list->cs);

    LIST_ITEM* item = (LIST_ITEM*)malloc(sizeof(LIST_ITEM));
    if (item == NULL)
    {
        cout << "add_list_front() failed: out of memory" << endl;
        LeaveCriticalSection(&list->cs);
        return;
    }

    item->data = data.data;
    item->next = NULL;

    if (list->count == 0)
    {
        list->head = item;
        list->tail = item;
    }
    else
    {
        item->next = list->head;
        list->head = item;
    }

    list->count++;
    LeaveCriticalSection(&list->cs);
}

// Dodavanje elementa na kraj liste
void add_list_back(LIST* list, LIST_ITEM data)
{
    if (list == NULL)
    {
        cout << "add_list_back() failed: list is NULL" << endl;
        return;
    }

    EnterCriticalSection(&list->cs);

    LIST_ITEM* item = (LIST_ITEM*)malloc(sizeof(LIST_ITEM));
    if (item == NULL)
    {
        cout << "add_list_back() failed: out of memory" << endl;
        LeaveCriticalSection(&list->cs);
        return;
    }

    item->data = data.data;
    item->next = NULL;

    if (list->count == 0)
    {
        list->head = item;
        list->tail = item;
    }
    else
    {
        list->tail->next = item;
        list->tail = item;
    }

    list->count++;
    LeaveCriticalSection(&list->cs);
}

// Dohvatanje stavke sa određenog indeksa
LIST_ITEM* get_list_item(LIST* list, int index)
{
    if (list == NULL)
    {
        cout << "get_list_item() failed: list is NULL" << endl;
        return NULL;
    }

    if (index < 0 || index >= list->count)
    {
        cout << "get_list_item() failed: index out of range" << endl;
        return NULL;
    }

    LIST_ITEM* item = list->head;
    for (int i = 0; i < index; i++)
    {
        item = item->next;
    }

    return item;
}

// Uklanjanje stavke sa određenog indeksa
bool remove_from_list(LIST* list, int index)
{
    if (list == NULL)
    {
        cout << "remove_from_list(): list is NULL" << endl;
        return true;
    }

    if (index < 0 || index >= list->count)
    {
        cout << "remove_from_list() failed: index out of range" << endl;
        return false;
    }

    EnterCriticalSection(&list->cs);

    LIST_ITEM* item = list->head;
    LIST_ITEM* prev = NULL;
    for (int i = 0; i < index; i++)
    {
        prev = item;
        item = item->next;
    }

    if (prev == NULL)
    {
        list->head = item->next;
    }
    else
    {
        prev->next = item->next;
    }

    if (item == list->tail)
    {
        list->tail = prev;
    }

    closesocket(item->data);  // Korišćenje closesocket za socket
    free(item);
    item = NULL;
    list->count--;

    LeaveCriticalSection(&list->cs);
    return true;
}

// Čišćenje liste
bool clear_list(LIST* list)
{
    if (list != NULL)
    {
        EnterCriticalSection(&list->cs);
        while (list->count > 0)
        {
            if (!remove_from_list(list, 0))
            {
                cout << "[WARN] clear_list() failed: failed to remove element from the list" << endl;
            }
        }
        LeaveCriticalSection(&list->cs);
    }

    return true;
}

// Oslobađanje resursa liste
bool free_list(LIST** list)
{
    if (list == NULL)
    {
        return true;
    }

    if (*list == NULL)
    {
        return true;
    }

    if ((*list)->count != 0)
    {
        EnterCriticalSection(&(*list)->cs);
        while ((*list)->head != (*list)->tail)
        {
            LIST_ITEM* item = (*list)->head;
            (*list)->head = item->next;
            shutdown(item->data, 2);        // Isključi socket pre nego što ga zatvoriš
            closesocket(item->data);        // Zatvori socket
            free(item);
            (*list)->count--;
        }
        free((*list)->head);
        (*list)->head = NULL;
        (*list)->tail = NULL;
        (*list)->count = 0;
        LeaveCriticalSection(&(*list)->cs);
    }

    DeleteCriticalSection(&(*list)->cs);
    free(*list);
    return true;
}

// Ispisivanje liste
void print_list(LIST* list)
{
    if (list == NULL)
    {
        cout << "print_list() failed: list is NULL" << endl;
        return;
    }

    if (list->count <= 0)
    {
        cout << "print_list(): list is empty" << endl;
        return;
    }

    LIST_ITEM* item = list->head;
    cout << "-------- List --------" << endl;
    cout << "List count: " << list->count << endl << "[";
    while (item != NULL)
    {
        cout << "Socket: " << item->data << ", ";
        item = item->next;
    }
    cout << "]" << endl;
    cout << "----------------------" << endl;
}
