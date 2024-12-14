#pragma once

#include "queueLBtoWorker.h"
#include <iostream>  // Za korišćenje cout
using namespace std;  // Omogućava direktno korišćenje std objekata

// Kreira red sa datim kapacitetom
QUEUE* init_queue(int capacity) {
    QUEUE* q = (QUEUE*)malloc(sizeof(QUEUE));  // Alocira memoriju za red
    if (!q) {
        cout << "Memory allocation failed for queue." << endl;
        return NULL;  // Ako alokacija ne uspe, vraća NULL
    }

    // Inicijalizacija kritične sekcije za sinhronizaciju u više niti
    InitializeCriticalSection(&q->cs);

    q->capacity = capacity;  // Postavlja kapacitet reda
    q->front = 0;            // Inicijalizuje front na 0
    q->rear = -1;            // Inicijalizuje rear na -1 (red je prazan)
    q->currentSize = 0;      // Inicijalizuje trenutnu veličinu reda na 0
    q->elements = (QUEUEELEMENT*)malloc(sizeof(QUEUEELEMENT) * capacity);  // Alocira memoriju za elemente reda
    if (!q->elements) {
        cout << "Memory allocation failed for queue elements." << endl;
        free(q);  // Oslobađa prethodno alociranu memoriju za red
        return NULL;  // Ako alokacija ne uspe, vraća NULL
    }

    return q;  // Vraća pokazivač na kreirani red
}

// Proverava da li je red pun
int is_queue_full(QUEUE* q) {
    return q->currentSize == q->capacity;
}

// Proverava da li je red prazan
int is_queue_empty(QUEUE* q) {
    return q->currentSize == 0;
}

// Dodaje element u red
void enqueue(QUEUE* q, QUEUEELEMENT* element) {
    EnterCriticalSection(&q->cs);  // Ulazi u kritičnu sekciju

    if (is_queue_full(q)) {
        cout << "Queue is full!" << endl;
        LeaveCriticalSection(&q->cs);  // Odlazi iz kritične sekcije
        return;
    }

    // Dodavanje elementa u red na poziciju rear
    q->rear = (q->rear + 1) % q->capacity;
    q->elements[q->rear] = *element;  // Kopiramo podatke u red
    q->currentSize++;  // Povećava trenutnu veličinu reda

    LeaveCriticalSection(&q->cs);  // Odlazi iz kritične sekcije
}

// Uklanja element sa početka reda i vraća ga
QUEUEELEMENT* dequeue(QUEUE* q) {
    EnterCriticalSection(&q->cs);  // Ulazi u kritičnu sekciju

    if (is_queue_empty(q)) {
        cout << "Queue is empty!" << endl;
        LeaveCriticalSection(&q->cs);  // Odlazi iz kritične sekcije
        return nullptr;  // Vraća NULL ako je red prazan
    }

    // Kreiramo pokazivač na element koji treba da se vrati
    QUEUEELEMENT* removedElement = (QUEUEELEMENT*)malloc(sizeof(QUEUEELEMENT));
    if (!removedElement) {
        cout << "Memory allocation failed for removed element." << endl;
        LeaveCriticalSection(&q->cs);  // Odlazi iz kritične sekcije
        return nullptr;  // Vraća NULL ako alokacija ne uspe
    }

    // Kopiramo element sa početka reda
    *removedElement = q->elements[q->front];

    // Pomeri front za jedan (cirkularni red)
    q->front = (q->front + 1) % q->capacity;
    q->currentSize--;  // Smanjuje trenutnu veličinu reda

    LeaveCriticalSection(&q->cs);  // Odlazi iz kritične sekcije

    return removedElement;  // Vraća pokazivač na izbačeni element
}

// Ispisuje sadržaj reda
void print_queue(QUEUE* q) {
    EnterCriticalSection(&q->cs);  // Ulazi u kritičnu sekciju

    if (is_queue_empty(q)) {
        cout << "Queue is empty!" << endl;
    }
    else {
        cout << "Queue contents:" << endl;
        for (int i = 0; i < q->currentSize; i++) {
            int index = (q->front + i) % q->capacity;  // Cirkularni pristup
            cout << "Client: " << q->elements[index].clientName << ", Data: " << q->elements[index].data << endl;
        }
    }

    LeaveCriticalSection(&q->cs);  // Odlazi iz kritične sekcije
}

// Vraća trenutnu veličinu reda
int get_current_size_queue(QUEUE* q) {
    return q->currentSize;
}

// Vraća kapacitet reda
int get_capacity_queue(QUEUE* q) {
    return q->capacity;
}

// Briše red i oslobađa memoriju
// Briše red i oslobađa memoriju
void delete_queue(QUEUE* q) {
    if (q == NULL) {
        return;  // Ako je pokazivač NULL, nema šta da se briše
    }

    EnterCriticalSection(&q->cs);  // Ulazi u kritičnu sekciju

    // Oslobađa memoriju za elemente reda
    free(q->elements);

    LeaveCriticalSection(&q->cs);  // Odlazi iz kritične sekcije

    // Briše kritičnu sekciju
    DeleteCriticalSection(&q->cs);

    // Oslobađa memoriju za samu strukturu reda
    free(q);
}


// Kreira i inicijalizuje QUEUEELEMENT
QUEUEELEMENT* create_queue_element(const char* clientName, const char* data) {
    // Alociramo memoriju za QUEUEELEMENT
    QUEUEELEMENT* newElement = (QUEUEELEMENT*)malloc(sizeof(QUEUEELEMENT));
    if (!newElement) {
        cout << "Memory allocation failed for queue element." << endl;
        return NULL;  // Ako alokacija ne uspe, vraća NULL
    }

    // Alociramo memoriju za clientName i data, kopiramo stringove
    newElement->clientName = (char*)malloc(strlen(clientName) + 1);
    if (!newElement->clientName) {
        cout << "Memory allocation failed for clientName." << endl;
        free(newElement);  // Oslobađamo prethodno alociranu memoriju
        return NULL;
    }
    // Sigurnije kopiranje stringa koristeći strcpy_s
    strcpy_s(newElement->clientName, strlen(clientName) + 1, clientName);

    newElement->data = (char*)malloc(strlen(data) + 1);
    if (!newElement->data) {
        cout << "Memory allocation failed for data." << endl;
        free(newElement->clientName);  // Oslobađamo prethodno alociranu memoriju
        free(newElement);  // Oslobađamo prethodno alociranu memoriju za element
        return NULL;
    }
    // Sigurnije kopiranje stringa koristeći strcpy_s
    strcpy_s(newElement->data, strlen(data) + 1, data);

    return newElement;  // Vraća pokazivač na kreirani element
}