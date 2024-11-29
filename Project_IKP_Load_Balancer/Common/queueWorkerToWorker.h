#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <windows.h>

// Struktura za element reda (PORT_QUEUEELEMENT)
typedef struct {
    char* clientName;   // Ime klijenta
    char* data;         // Poruka koja treba da se pošalje
    uint16_t* ports;    // Niz portova
    size_t numPorts;    // Broj portova
} PORT_QUEUEELEMENT;

// Struktura za red (PORT_QUEUE)
typedef struct {
    CRITICAL_SECTION cs;  // Za sinhronizaciju pristupa u više niti
    int front, rear, currentSize;  // Indeksi fronta, rear-a i trenutna veličina reda
    int capacity;  // Kapacitet reda
    PORT_QUEUEELEMENT* elements;  // Niz elemenata reda
} PORT_QUEUE;

// Funkcije za rad sa redom
PORT_QUEUE* init_port_queue(int capacity);  // Kreira red sa datim kapacitetom
int is_port_queue_full(PORT_QUEUE* q);      // Proverava da li je red pun
int is_port_queue_empty(PORT_QUEUE* q);     // Proverava da li je red prazan
void enqueue_port(PORT_QUEUE* q, PORT_QUEUEELEMENT* element);  // Dodaje element u red
PORT_QUEUEELEMENT* dequeue_port(PORT_QUEUE* q);  // Uklanja element sa početka reda
void print_port_queue(PORT_QUEUE* q);        // Ispisuje sadržaj reda
int get_current_size_port_queue(PORT_QUEUE* q);  // Vraća trenutnu veličinu reda
int get_capacity_port_queue(PORT_QUEUE* q);    // Vraća kapacitet reda
void delete_port_queue(PORT_QUEUE* q);       // Briše red i oslobađa memoriju
PORT_QUEUEELEMENT* create_port_queue_element(const char* clientName, const char* data, const uint16_t* ports, size_t numPorts);  // Kreira element reda
