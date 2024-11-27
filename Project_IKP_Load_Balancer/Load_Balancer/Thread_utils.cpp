#include "thread_utils.h"
#include "networking_utils.h"

DWORD WINAPI AcceptConnectionsThread(LPVOID lpParam) {
    ThreadParams* params = (ThreadParams*)lpParam;

    SOCKET serverSocket = params->serverSocket;
    fd_set* readSet = params->readSet;
    fd_set* exceptSet = params->exceptSet;
    int maxFd = params->maxFd;

    while (1) {
        FD_ZERO(readSet);
        FD_ZERO(exceptSet);
        FD_SET(serverSocket, readSet);
        FD_SET(serverSocket, exceptSet);

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int selectResult = select(maxFd + 1, readSet, NULL, exceptSet, &tv);
        if (selectResult > 0 && FD_ISSET(serverSocket, readSet)) {
            struct sockaddr_in clientAddr;
            int clientAddrSize = sizeof(clientAddr);

            SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
            if (clientSocket != INVALID_SOCKET) {
                printf("New client/worker connected\n");

                // Process the new client request
                process_new_request(clientSocket);
            }
            else {
                fprintf(stderr, "Failed to accept connection\n");
            }
        }
    }

    return 0;
}
