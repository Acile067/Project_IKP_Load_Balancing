// Load_Balancer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <winsock.h>
using namespace std;
#define PORT 5059

int nSocket;
struct sockaddr_in srv;
fd_set fr, fw, fe;  //Read,Write,Exception
int nMaxFd;
int nArrClient[5];                   //TODO HashMap

void ProcessNewMessage(int nClientSocket) 
{
    cout << endl << "Procesing message from client: " << nClientSocket;
    char buffer[256 + 1] = { 0, };
    int nRet = recv(nClientSocket, buffer, 256, 0);
    if (nRet < 0)
    {
        cout << endl << "Something bad happen. Closing Socket";
        closesocket(nClientSocket);
        for (int nIndex = 0; nIndex < 5; nIndex++)              //TODO HashMap
        {
            if (nArrClient[nIndex] == nClientSocket)
            {
                nArrClient[nIndex] = 0;
                break;
            }
        }
    }
    else
    {
        cout << endl << "CLIENT SAY: " << buffer;
        send(nClientSocket, "SERVER: Got message!", 21, 0);
        cout << endl << "******************";
    }

}

void ProcessTheNewRequest()
{
    //New Connection Request
    if (FD_ISSET(nSocket, &fr))
    {
        int nLen = sizeof(struct sockaddr);
        int nClientSocket = accept(nSocket, NULL, &nLen);
        if (nClientSocket > 0)
        {
            //Put it into client fd_set             TODO HashMap
            int nIndex = 0;
            for (nIndex = 0; nIndex < 5; nIndex++)
            {
                if (nArrClient[nIndex] == 0) 
                {
                    nArrClient[nIndex] = nClientSocket;
                    send(nClientSocket, "SERVER: You are connected", 26, 0);
                    break;
                }
            }
            if (nIndex == 5)
            {
                cout << endl << "No More Space For New Connection";
            }
        }
    }
    else
    {
        for (int nIndex = 0; nIndex < 5; nIndex++)          //TODO HashMap
        {
            if (FD_ISSET(nArrClient[nIndex], &fr))
            {
                //New Message From Client
                ProcessNewMessage(nArrClient[nIndex]);
            }
        }
    }
}

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
    nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (nSocket < 0)
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
    srv.sin_addr.s_addr = INADDR_ANY;
    memset(&(srv.sin_zero), 0, 8);

    //Blocking or no blocking
    u_long optval = 0; //0 blocking !=0 non blocking
    nRet = ioctlsocket(nSocket, FIONBIO, &optval);
    if (nRet != 0)
    {
        cout << endl << "ioctlsocket call failed";
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << endl << "ioctlsocket call passed";
    }

    //setsockopt
    int nOptVal = 0;
    int nOptLen = sizeof(nOptVal);
    nRet = setsockopt(nSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&nOptVal, nOptLen);
    if (!nRet)
    {
        cout << endl << "setsockopt success";
    }
    else
    {
        cout << endl << "setsockopt failed";
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    //Binding
    nRet = bind(nSocket, (sockaddr*)&srv, sizeof(sockaddr));
    if (nRet < 0)
    {
        cout << endl << "Binding Failed";
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << endl << "Binding Succes";
    }

    //Listen for clients requests
    nRet = listen(nSocket, 10); //10 requests
    if (nRet < 0)
    {
        cout << endl << "Listen Failed";
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << endl << "Listen Succes";
    }

    //Keep waitnig for new clients
    nMaxFd = nSocket;
    struct timeval tv;
    tv.tv_sec = 1;

    while (1)
    {
        FD_ZERO(&fr);
        FD_ZERO(&fw);
        FD_ZERO(&fe);

        FD_SET(nSocket, &fr);
        FD_SET(nSocket, &fe);

        for (int nIndex = 0; nIndex < 5; nIndex++)           //TODO HashMap za ovo 5
        {
            if (nArrClient[nIndex] != 0)
            {
                FD_SET(nArrClient[nIndex], &fr);
                FD_SET(nArrClient[nIndex], &fe);
            }
        }
        //cout << endl << "Befor select call:" << fr.fd_count;

        nRet = select(nMaxFd + 1, &fr, &fw, &fe, &tv);
        if (nRet > 0)
        {
            //someone connected
            cout << endl << "Someone connected!";       //comment later
            //Process request
            ProcessTheNewRequest();

            //break;
        }
        else if (nRet == 0)
        {
            //no connections
            //cout << endl << "Nothing on Port";
        }
        else
        {
            //failed
            cout << endl << "Failed";
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        //cout << endl << "After select call:" << fr.fd_count;
        //Sleep(1000);
    }
    
}
