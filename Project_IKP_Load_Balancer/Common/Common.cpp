#pragma once
#include "common.h"
#include <winsock.h>
#include <stdio.h>

// Inicijalizacija Winsock 1.x
bool InitializeWinsock()
{
    WSADATA wsaData;
    // Inicijalizacija Winsock-a za verziju 1.1
    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
        cout << "WSAStartup() failed with error: " << WSAGetLastError() << endl;
        return false;
    }
    return true;
}

// Funkcija za povezivanje na server
SOCKET connect(short port)
{
    SOCKET sock = INVALID_SOCKET;

    //if (InitializeWinsock() == false) {
    //    printf("Failed to initialize winsock\n");
    //    return INVALID_SOCKET;
    //}

    //sockaddr_in addr;
    //int addr_len = sizeof(addr);

    //// Kreiranje soketa
    //sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //if (sock == INVALID_SOCKET) {
    //    printf("socket() failed with error: %d\n", WSAGetLastError());
    //    return INVALID_SOCKET;
    //}

    //// Postavljanje servera i porta
    //addr.sin_family = AF_INET;
    //addr.sin_port = htons(port);
    //addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);  // Pretvori server adresu

    //// Povezivanje sa serverom
    //if (connect(sock, (sockaddr*)&addr, addr_len) == SOCKET_ERROR) {
    //    printf("connect() failed with error: %d\n", WSAGetLastError());
    //    closesocket(sock);  // Zatvori soket u slu?aju greške
    //    return INVALID_SOCKET;
    //}

    return sock;  // Vra?a soket ako je povezivanje uspešno
}