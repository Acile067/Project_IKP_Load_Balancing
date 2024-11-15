#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <windows.h>

// Struktura za element reda
typedef struct {
    char* clientName;  // Ime klijenta
    char* data;        // Podaci koji se čuvaju u redu
} QUEUEELEMENT;

// Struktura za red
typedef struct {
    CRITICAL_SECTION cs;  // Za sinhronizaciju pristupa u više niti
    int front, rear, currentSize;  // Indeksi fronta, rear-a i trenutna veličina reda
    int capacity;  // Kapacitet reda
    QUEUEELEMENT* elements;  // Niz elemenata reda
} QUEUE;  // Ne zaboravite tačku i zarez na kraju definicije strukture

// Funkcije za rad sa redom
QUEUE* init_queue(int capacity);  // Kreira red sa datim kapacitetom
int is_queue_full(QUEUE* q);      // Proverava da li je red pun
int is_queue_empty(QUEUE* q);     // Proverava da li je red prazan
void enqueue(QUEUE* q, QUEUEELEMENT* element);  // Dodaje element u red
QUEUEELEMENT* dequeue(QUEUE* q);  // Uklanja element sa početka reda
void print_queue(QUEUE* q);        // Ispisuje sadržaj reda
int get_current_size_queue(QUEUE* q);  // Vraća trenutnu veličinu reda
int get_capacity_queue(QUEUE* q);    // Vraća kapacitet reda
void delete_queue(QUEUE* q);       // Briše red i oslobađa memoriju
QUEUEELEMENT* create_queue_element(const char* clientName, const char* data);