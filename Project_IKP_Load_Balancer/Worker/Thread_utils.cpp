#include "thread_utils.h"
#include "networking_utils.h"

DWORD WINAPI ProcessLBMessage(LPVOID lpParam)
{
    // Preuzimanje socket-a iz lpParam
    SOCKET workerSocket = *(SOCKET*)lpParam;

    fd_set read_fds;           // Skup soketa za praćenje događaja za čitanje
    struct timeval timeout;    // Tajmaut za `select`
    int nRet;

    while (1)
    {
        FD_ZERO(&read_fds);
        FD_SET(workerSocket, &read_fds);

        // Postavljanje tajmauta (1 sekunda)
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        // Praćenje događaja na soketu
        nRet = select(0, &read_fds, NULL, NULL, &timeout);
        if (nRet > 0)
        {
            if (FD_ISSET(workerSocket, &read_fds)) {
                // Pozivanje funkcije za prijem i obradu podataka
                receive_combined_data(workerSocket);
            }
        }
        else if (nRet < 0) {
            // Obrada greške `select`
            printf("Error in select: %d\n", WSAGetLastError());
            break;
        }
    }

    return 0;
}
