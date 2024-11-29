#pragma once

#include "queueWorkerToWorker.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <windows.h>
#include <iostream>

using namespace std;

// Funkcija za inicijalizaciju reda sa datim kapacitetom
PORT_QUEUE* init_port_queue(int capacity) {
    // Alociramo memoriju za red
    PORT_QUEUE* queue = (PORT_QUEUE*)malloc(sizeof(PORT_QUEUE));
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
    queue->elements = (PORT_QUEUEELEMENT*)malloc(sizeof(PORT_QUEUEELEMENT) * capacity);
    if (queue->elements == nullptr) {
        cout << "Memory allocation failed for queue elements." << endl;
        free(queue);  // Oslobađa prethodno alociranu memoriju za red
        return nullptr;  // Ako alokacija ne uspe, vraća nullptr
    }

    // Inicijalizacija svakog elementa u redu
    for (int i = 0; i < capacity; i++) {
        queue->elements[i].clientName = nullptr;
        queue->elements[i].data = nullptr;
        queue->elements[i].ports = nullptr;
        queue->elements[i].numPorts = 0;
    }

    return queue;  // Vraća pokazivač na kreirani red
}

// Funkcija za proveru da li je red pun
int is_port_queue_full(PORT_QUEUE* q) {
    return q->currentSize == q->capacity;
}

// Funkcija za proveru da li je red prazan
int is_port_queue_empty(PORT_QUEUE* q) {
    return q->currentSize == 0;
}

// Funkcija za dodavanje elementa u red
void enqueue_port(PORT_QUEUE* q, PORT_QUEUEELEMENT* element) {
    EnterCriticalSection(&q->cs);  // Ulazimo u kritičnu sekciju

    // Proveravamo da li je red pun
    if (is_port_queue_full(q)) {
        cerr << "Queue is full, cannot enqueue!" << endl;
        LeaveCriticalSection(&q->cs);  // Napuštamo kritičnu sekciju
        return;
    }

    // Povećavamo rear i dodajemo element
    q->rear = (q->rear + 1) % q->capacity;
    q->elements[q->rear] = *element;
    q->currentSize++;

    LeaveCriticalSection(&q->cs);  // Napuštamo kritičnu sekciju
}

// Funkcija za uklanjanje elementa sa početka reda
PORT_QUEUEELEMENT* dequeue_port(PORT_QUEUE* q) {
    EnterCriticalSection(&q->cs);  // Ulazimo u kritičnu sekciju

    // Proveravamo da li je red prazan
    if (is_port_queue_empty(q)) {
        cerr << "Queue is empty, cannot dequeue!" << endl;
        LeaveCriticalSection(&q->cs);  // Napuštamo kritičnu sekciju
        return nullptr;  // Red je prazan
    }

    // Uklanjamo element sa fronta
    PORT_QUEUEELEMENT* removedElement = &q->elements[q->front];

    // Pomera front i ažurira veličinu
    q->front = (q->front + 1) % q->capacity;
    q->currentSize--;

    LeaveCriticalSection(&q->cs);  // Napuštamo kritičnu sekciju

    return removedElement;  // Vraćamo uklonjeni element
}

// Funkcija za ispis sadržaja reda
void print_port_queue(PORT_QUEUE* q) {
    EnterCriticalSection(&q->cs);  // Ulazimo u kritičnu sekciju

    if (is_port_queue_empty(q)) {
        cout << "Queue is empty!" << endl;
    }
    else {
        cout << "Queue elements:" << endl;
        int i = q->front;
        int count = 0;

        // Ispisujemo sve elemente reda
        while (count < q->currentSize) {
            PORT_QUEUEELEMENT* element = &q->elements[i];
            cout << "Client Name: " << element->clientName << ", Data: " << element->data << ", Ports: ";

            // Ispisujemo portove
            for (size_t j = 0; j < element->numPorts; j++) {
                cout << element->ports[j] << " ";
            }
            cout << endl;
            i = (i + 1) % q->capacity;
            count++;
        }
    }

    LeaveCriticalSection(&q->cs);  // Napuštamo kritičnu sekciju
}

// Funkcija za vraćanje trenutne veličine reda
int get_current_size_port_queue(PORT_QUEUE* q) {
    return q->currentSize;
}

// Funkcija za vraćanje kapaciteta reda
int get_capacity_port_queue(PORT_QUEUE* q) {
    return q->capacity;
}

// Funkcija za brisanje reda i oslobađanje memorije
void delete_port_queue(PORT_QUEUE* q) {
    if (q == nullptr) {
        return;  // Ako je pokazivač NULL, nema šta da se briše
    }

    EnterCriticalSection(&q->cs);  // Ulazi u kritičnu sekciju

    // Oslobađamo memoriju za svaki element reda
    for (int i = 0; i < q->capacity; i++) {
        // Proveravamo da li su članovi inicijalizovani pre nego što ih oslobodimo
        if (q->elements[i].clientName != nullptr) {
            free(q->elements[i].clientName);
            q->elements[i].clientName = nullptr;
        }
        if (q->elements[i].data != nullptr) {
            free(q->elements[i].data);
            q->elements[i].data = nullptr;
        }
        if (q->elements[i].ports != nullptr) {
            free(q->elements[i].ports);
            q->elements[i].ports = nullptr;
        }
    }

    LeaveCriticalSection(&q->cs);  // Napuštamo kritičnu sekciju

    // Oslobađamo memoriju za niz elemenata
    free(q->elements);

    // Uništavamo kritičnu sekciju
    DeleteCriticalSection(&q->cs);

    // Oslobađamo memoriju za samu strukturu reda
    free(q);
}

// Funkcija za kreiranje elementa reda
PORT_QUEUEELEMENT* create_port_queue_element(const char* clientName, const char* data, const uint16_t* ports, size_t numPorts) {
    PORT_QUEUEELEMENT* element = (PORT_QUEUEELEMENT*)malloc(sizeof(PORT_QUEUEELEMENT));
    if (element == nullptr) {
        cerr << "Error: Unable to allocate memory for the queue element!" << endl;
        return nullptr;
    }

    // Alociramo i kopiramo podatke
    element->clientName = _strdup(clientName);  // Kopira ime klijenta
    element->data = _strdup(data);  // Kopira podatke
    element->ports = (uint16_t*)malloc(sizeof(uint16_t) * numPorts);  // Alociramo memoriju za portove
    memcpy(element->ports, ports, sizeof(uint16_t) * numPorts);  // Kopiramo portove
    element->numPorts = numPorts;  // Broj portova

    return element;
}
