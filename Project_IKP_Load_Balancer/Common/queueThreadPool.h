#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <windows.h>
#include <stdint.h>

// Struktura za element reda
typedef struct {
    char* clientName;  // Ime klijenta
    char* data;        // Podaci koji se čuvaju u redu
    uint16_t targetPort;  // Port za ciljnu komunikaciju
} THREAD_QUEUEELEMENT;

// Struktura za red
typedef struct {
    CRITICAL_SECTION cs;  // Za sinhronizaciju pristupa u više niti
    int front, rear, currentSize;  // Indeksi fronta, rear-a i trenutna veličina reda
    int capacity;  // Kapacitet reda
    THREAD_QUEUEELEMENT* elements;  // Niz elemenata reda
} THREAD_QUEUE;

// Funkcije za rad sa redom
THREAD_QUEUE* init_thread_queue(int capacity);  // Kreira red sa datim kapacitetom
int is_thread_queue_full(THREAD_QUEUE* q);      // Proverava da li je red pun
int is_thread_queue_empty(THREAD_QUEUE* q);     // Proverava da li je red prazan
void enqueue_thread_queue(THREAD_QUEUE* q, THREAD_QUEUEELEMENT* element);  // Dodaje element u red
THREAD_QUEUEELEMENT* dequeue_thread_queue(THREAD_QUEUE* q);  // Uklanja element sa početka reda
void print_thread_queue(THREAD_QUEUE* q);       // Ispisuje sadržaj reda
int get_current_size_thread_queue(THREAD_QUEUE* q);  // Vraća trenutnu veličinu reda
int get_capacity_thread_queue(THREAD_QUEUE* q);  // Vraća kapacitet reda
void delete_thread_queue(THREAD_QUEUE* q);       // Briše red i oslobađa memoriju
THREAD_QUEUEELEMENT* create_thread_queue_element(const char* clientName, const char* data, uint16_t targetPort);