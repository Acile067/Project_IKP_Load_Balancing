#pragma once

#include <winsock.h>  // Koriš?enje Winsock1 (umesto Winsock2.h)
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;
#pragma comment(lib, "wsock32.lib")  // Za Winsock1, koristi wsock32.lib

/// <summary>
/// Connect to server
/// </summary>
/// <param name="port"> - Connect port</param>
/// <param name="server_address"> - server address</param>
/// <returns>Socket handle if successful, otherwise INVALID_SOCKET</returns>
SOCKET connect(short port, const char* server_address);

/// <summary>
/// Initialize Winsock library
/// </summary>
/// <returns>True if successful, otherwise false</returns>
bool InitializeWinsock();