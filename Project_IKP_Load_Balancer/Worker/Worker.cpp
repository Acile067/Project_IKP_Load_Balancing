// Worker.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <winsock.h>
using namespace std;
#define PORT 5059

int nWorkerSocket;
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
    nWorkerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (nWorkerSocket < 0)
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
    nRet = connect(nWorkerSocket, (struct sockaddr*)&srv, sizeof(srv));
    if (nRet < 0)
    {
        cout << endl << "Connect Failed";
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << endl << "WORKER Connect Success";
        //Server accepted
        char buffer[255] = { 0, };

        recv(nWorkerSocket, buffer, 255, 0);
        cout << endl << buffer;
    }

    

    cout << endl << "Press any to exit... Worker is Running";
    char exit = getchar();
}

