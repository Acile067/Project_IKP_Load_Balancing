// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <winsock.h>
using namespace std;
#define PORT 5059

int nClientSocket;
struct sockaddr_in srv;

int main()
{
    int nRet = 0;
    //Init WSA 
    WSADATA ws;
    if (WSAStartup(MAKEWORD(2, 2), &ws) < 0)
    {
        cout << endl << "WSA Failed";
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << endl << "WSA Succes";
    }

    //Init Socket
    nClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (nClientSocket < 0)
    {
        cout << endl << "Socket not open";
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << endl << "Socket open";
    }

    //Init env for sockaddr structure
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    srv.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(&(srv.sin_zero), 0, 8);

    //connect
    nRet = connect(nClientSocket, (struct sockaddr*)&srv, sizeof(srv));
    if (nRet < 0)
    {
        cout << endl << "Connect Failed";
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    else
    {
        send(nClientSocket, "CLIENT", 6, 0);

        cout << endl << "CLIENT Connect Success";
        //Server accepted
        char buffer[255] = { 0, };

        recv(nClientSocket, buffer, 255, 0);
        cout << endl << buffer;

        //Messages
        cout << endl << "Send Message: ";

        while (1) {
            cout << endl << "Enter message or type 'end' to exit: ";
            fgets(buffer, 256, stdin);

            // Remove the newline character from the buffer
            buffer[strcspn(buffer, "\n")] = '\0';

            if (strcmp(buffer, "end") == 0) {
                cout << endl << "END";
                break;
            }

            // Send the message to the server
            send(nClientSocket, buffer, strlen(buffer), 0);

            // Receive the response from the server
            int bytesReceived = recv(nClientSocket, buffer, 256, 0);
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0'; // Null-terminate the received data
                cout << endl << buffer;
            }
            else {
                cout << endl << "Connection closed or error occurred.";
                break;
            }
        }

        closesocket(nClientSocket);
    }
}
