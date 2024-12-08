#pragma once

#include "queueThreadPool.h"
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <windows.h>
#include <iostream>

using namespace std;

// Funkcija za inicijalizaciju reda sa datim kapacitetom
THREAD_QUEUE* init_thread_queue(int capacity) {
    // Alociramo memoriju za red
    THREAD_QUEUE* queue = (THREAD_QUEUE*)malloc(sizeof(THREAD_QUEUE));
    if (queue == nullptr) {
        cout << "Memory allocation failed for the queue." << endl;
        return nullptr;  // Ako alokacija ne uspe, vraća nullptr
    }

    // Inicijalizacija kritične sekcije za sinhronizaciju u više niti
    InitializeCriticalSection(&queue->cs);

    queue->capacity = capacity;         // Postavlja kapacitet reda
    queue->front = 0;                   // Inicijalizuje front na 0
    queue->rear = -1;                   // Inicijalizuje rear na -1 (red je prazan)
    queue->currentSize = 0;             // Inicijalizuje trenutnu veličinu reda

    // Alocira memoriju za elemente reda
    queue->elements = (THREAD_QUEUEELEMENT*)malloc(sizeof(THREAD_QUEUEELEMENT) * capacity);
    if (queue->elements == nullptr) {
        cout << "Memory allocation failed for queue elements." << endl;
        free(queue);  // Oslobađa prethodno alociranu memoriju za red
        return nullptr;  // Ako alokacija ne uspe, vraća nullptr
    }

    // Inicijalizacija svakog elementa u redu
    for (int i = 0; i < capacity; i++) {
        queue->elements[i].clientName = nullptr;
        queue->elements[i].data = nullptr;
        queue->elements[i].targetPort = 0;
    }

    return queue;  // Vraća pokazivač na kreirani red
}

// Proverava da li je red pun
int is_thread_queue_full(THREAD_QUEUE* q) {
    return q->currentSize == q->capacity;
}

// Proverava da li je red prazan
int is_thread_queue_empty(THREAD_QUEUE* q) {
    return q->currentSize == 0;
}

// Dodaje element u red
void enqueue_thread_queue(THREAD_QUEUE* q, THREAD_QUEUEELEMENT* element) {
    EnterCriticalSection(&q->cs);

    if (is_thread_queue_full(q)) {
        cout << "Queue is full! Cannot enqueue." << endl;
        LeaveCriticalSection(&q->cs);
        return;
    }

    q->rear = (q->rear + 1) % q->capacity;
    q->elements[q->rear] = *element;
    q->currentSize++;

    LeaveCriticalSection(&q->cs);
}

// Funkcija za uklanjanje elementa sa početka reda
THREAD_QUEUEELEMENT* dequeue_thread_queue(THREAD_QUEUE* q) {
    EnterCriticalSection(&q->cs);

    if (is_thread_queue_empty(q)) {
        LeaveCriticalSection(&q->cs);
        return nullptr;  // Red je prazan, vraća nullptr
    }

    // Alociraj memoriju za uklonjeni element
    THREAD_QUEUEELEMENT* removedElement = (THREAD_QUEUEELEMENT*)malloc(sizeof(THREAD_QUEUEELEMENT));
    if (removedElement != nullptr) {
        *removedElement = q->elements[q->front]; // Plitka kopija elementa
        q->front = (q->front + 1) % q->capacity; // Pomeri front na sledeći element
        q->currentSize--;
    }

    LeaveCriticalSection(&q->cs);

    return removedElement;  // Vraća pokazivač na kopirani element
}


// Ispisuje sadržaj reda
void print_thread_queue(THREAD_QUEUE* q) {
    EnterCriticalSection(&q->cs);

    cout << "Queue content:" << endl;
    for (int i = 0; i < q->currentSize; i++) {
        int index = (q->front + i) % q->capacity;
        cout << "ClientName: " << q->elements[index].clientName
            << ", Data: " << q->elements[index].data
            << ", TargetPort: " << q->elements[index].targetPort << endl;
    }

    LeaveCriticalSection(&q->cs);
}

// Vraća trenutnu veličinu reda
int get_current_size_thread_queue(THREAD_QUEUE* q) {
    return q->currentSize;
}

// Vraća kapacitet reda
int get_capacity_thread_queue(THREAD_QUEUE* q) {
    return q->capacity;
}

void delete_thread_queue(THREAD_QUEUE* q) {
    if (q == nullptr) {
        return;  // Ako je pokazivač nullptr, nema šta da se briše
    }

    EnterCriticalSection(&q->cs);  // Ulazi u kritičnu sekciju

    // Oslobađamo memoriju za svaki element reda
    for (int i = 0; i < q->capacity; i++) {
        if (q->elements[i].clientName != nullptr) {
            free(q->elements[i].clientName);
            q->elements[i].clientName = nullptr;
        }
        if (q->elements[i].data != nullptr) {
            free(q->elements[i].data);
            q->elements[i].data = nullptr;
        }
    }

    LeaveCriticalSection(&q->cs);  // Napuštamo kritičnu sekciju

    // Oslobađamo memoriju za niz elemenata
    free(q->elements);
    q->elements = nullptr;

    // Uništavamo kritičnu sekciju
    DeleteCriticalSection(&q->cs);

    // Oslobađamo memoriju za samu strukturu reda
    free(q);
    q = nullptr;
}



// Kreira novi element reda
THREAD_QUEUEELEMENT* create_thread_queue_element(const char* clientName, const char* data, uint16_t targetPort) {
    THREAD_QUEUEELEMENT* element = (THREAD_QUEUEELEMENT*)malloc(sizeof(THREAD_QUEUEELEMENT));
    element->clientName = _strdup(clientName);
    element->data = _strdup(data);
    element->targetPort = targetPort;
    return element;
}
