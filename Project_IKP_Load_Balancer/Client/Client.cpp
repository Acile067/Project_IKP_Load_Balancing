//// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include <winsock.h>
#include <cstring>  // Za memset
using namespace std;

#define PORT 5059

int nClientSocket;
struct sockaddr_in srv;

void cleanup() {
    // Zatvori soket i očisti resurse
    if (nClientSocket != INVALID_SOCKET) {
        closesocket(nClientSocket);
        nClientSocket = INVALID_SOCKET;
    }
    WSACleanup();
}

int main() {
    int nRet = 0;

    // Inicijalizacija Winsock-a
    WSADATA ws;
    if (WSAStartup(MAKEWORD(2, 2), &ws) < 0) {
        cout << "WSA Failed" << endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    else {
        cout << "WSA Success" << endl;
    }

    // Inicijalizacija soketa
    nClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (nClientSocket < 0) {
        cout << "Socket not open" << endl;
        cleanup();  // Očisti resurse
        exit(EXIT_FAILURE);
    }
    else {
        cout << "Socket open" << endl;
    }

    // Konfiguracija sockaddr strukture
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    srv.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(&(srv.sin_zero), 0, 8);

    // Povezivanje sa serverom
    nRet = connect(nClientSocket, (struct sockaddr*)&srv, sizeof(srv));
    if (nRet < 0) {
        cout << "Connect Failed" << endl;
        cleanup();  // Očisti resurse
        exit(EXIT_FAILURE);
    }
    else {
        send(nClientSocket, "CLIENT", 6, 0);
        cout << "CLIENT Connect Success" << endl;
    }

    // Očekuj poruke od servera
    char buffer[255] = { 0 };

    // Prima početnu poruku od servera
    int bytesReceived = recv(nClientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';  // Null-terminate the received data
        cout << "Server: " << buffer << endl;
    }
    else if (bytesReceived == 0) {
        cout << "Server closed the connection." << endl;
        cleanup();  // Očisti resurse
        exit(EXIT_FAILURE);
    }
    else {
        cout << "Error receiving message from server." << endl;
        cleanup();  // Očisti resurse
        exit(EXIT_FAILURE);
    }

    // Poruke od klijenta
    cout << "Send Message: " << endl;

    while (true) {
        cout << "Enter message or type 'end' to exit: ";
        fgets(buffer, sizeof(buffer), stdin);

        // Uklanjanje '\n' karaktera sa kraja poruke
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "end") == 0) {
            cout << "END" << endl;
            break;
        }

        // Šaljemo poruku serveru
        int bytesSent = send(nClientSocket, buffer, strlen(buffer), 0);
        if (bytesSent == SOCKET_ERROR) {
            cout << "Error sending message" << endl;
            break;
        }

        // Prima odgovor od servera
        bytesReceived = recv(nClientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0'; // Null-terminate the received data
            cout << "Server: " << buffer << endl;
        }
        else if (bytesReceived == 0) {
            cout << "Connection closed by server." << endl;
            break;
        }
        else {
            cout << "Connection error or closed." << endl;
            break;
        }
    }

    // Zatvori konekciju i očisti resurse
    cleanup();
    return 0;
}
