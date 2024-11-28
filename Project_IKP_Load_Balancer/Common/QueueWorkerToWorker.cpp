#pragma once

#include "queueWorkerToWorker.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Kreira red sa datim kapacitetom
PORT_QUEUE* init_port_queue(int capacity) {
    PORT_QUEUE* q = (PORT_QUEUE*)malloc(sizeof(PORT_QUEUE));
    if (q == NULL) {
        return NULL;  // Ako nije moguće alocirati memoriju za red
    }

    q->capacity = capacity;
    q->front = 0;
    q->rear = -1;
    q->currentSize = 0;
    q->elements = (PORT_QUEUEELEMENT*)malloc(capacity * sizeof(PORT_QUEUEELEMENT));

    if (q->elements == NULL) {
        free(q);
        return NULL;  // Ako nije moguće alocirati memoriju za elemente
    }

    InitializeCriticalSection(&q->cs);  // Inicijalizuje kritičnu sekciju za sinhronizaciju
    return q;
}

// Proverava da li je red pun
int is_port_queue_full(PORT_QUEUE* q) {
    return q->currentSize == q->capacity;
}

// Proverava da li je red prazan
int is_port_queue_empty(PORT_QUEUE* q) {
    return q->currentSize == 0;
}

// Dodaje element u red
void enqueue_port(PORT_QUEUE* q, PORT_QUEUEELEMENT* element) {
    EnterCriticalSection(&q->cs);  // Zauzima kritičnu sekciju

    if (is_port_queue_full(q)) {
        printf("Red je pun!\n");
        LeaveCriticalSection(&q->cs);
        return;
    }

    q->rear = (q->rear + 1) % q->capacity;  // Circularni raspored
    q->elements[q->rear] = *element;  // Dodaje element
    q->currentSize++;

    LeaveCriticalSection(&q->cs);  // Oslobađa kritičnu sekciju
}

// Uklanja element sa početka reda
PORT_QUEUEELEMENT* dequeue_port(PORT_QUEUE* q) {
    EnterCriticalSection(&q->cs);  // Zauzima kritičnu sekciju

    if (is_port_queue_empty(q)) {
        printf("Red je prazan!\n");
        LeaveCriticalSection(&q->cs);
        return NULL;
    }

    PORT_QUEUEELEMENT* element = &q->elements[q->front];
    q->front = (q->front + 1) % q->capacity;  // Circularni raspored
    q->currentSize--;

    LeaveCriticalSection(&q->cs);  // Oslobađa kritičnu sekciju
    return element;
}

// Ispisuje sadržaj reda
void print_port_queue(PORT_QUEUE* q) {
    EnterCriticalSection(&q->cs);  // Zauzima kritičnu sekciju

    if (is_port_queue_empty(q)) {
        printf("Red je prazan!\n");
    }
    else {
        printf("Sadrzaj reda:\n");
        int i = q->front;
        for (int j = 0; j < q->currentSize; j++) {
            printf("Klijent: %s\n", q->elements[i].clientName);
            printf("Portovi: ");
            for (size_t k = 0; k < q->elements[i].numPorts; k++) {
                printf("%d ", q->elements[i].ports[k]);
            }
            printf("\n");
            i = (i + 1) % q->capacity;  // Circularni raspored
        }
    }

    LeaveCriticalSection(&q->cs);  // Oslobađa kritičnu sekciju
}

// Vraća trenutnu veličinu reda
int get_current_size_port_queue(PORT_QUEUE* q) {
    return q->currentSize;
}

// Vraća kapacitet reda
int get_capacity_port_queue(PORT_QUEUE* q) {
    return q->capacity;
}

// Briše red i oslobađa memoriju
void delete_port_queue(PORT_QUEUE* q) {
    EnterCriticalSection(&q->cs);  // Zauzima kritičnu sekciju

    for (int i = 0; i < q->currentSize; i++) {
        free(q->elements[i].ports);  // Oslobađa memoriju za portove
        free(q->elements[i].clientName);  // Oslobađa memoriju za ime klijenta
    }

    free(q->elements);  // Oslobađa memoriju za elemente reda
    DeleteCriticalSection(&q->cs);  // Briše kritičnu sekciju
    free(q);  // Oslobađa memoriju za red
}

// Kreira element reda
PORT_QUEUEELEMENT* create_port_queue_element(const char* clientName, const uint16_t* ports, size_t numPorts) {
    PORT_QUEUEELEMENT* element = (PORT_QUEUEELEMENT*)malloc(sizeof(PORT_QUEUEELEMENT));
    if (element == NULL) {
        return NULL;  // Ako nije moguće alocirati memoriju
    }

    element->clientName = (char*)malloc(strlen(clientName) + 1);
    if (element->clientName == NULL) {
        free(element);
        return NULL;  // Ako nije moguće alocirati memoriju za ime klijenta
    }

    // Safe copy of clientName using strcpy_s
    if (strcpy_s(element->clientName, strlen(clientName) + 1, clientName) != 0) {
        free(element->clientName);
        free(element);
        return NULL;  // Ako nije moguće kopirati ime klijenta
    }

    element->ports = (uint16_t*)malloc(numPorts * sizeof(uint16_t));
    if (element->ports == NULL) {
        free(element->clientName);
        free(element);
        return NULL;  // Ako nije moguće alocirati memoriju za portove
    }
    memcpy(element->ports, ports, numPorts * sizeof(uint16_t));
    element->numPorts = numPorts;

    return element;
}

