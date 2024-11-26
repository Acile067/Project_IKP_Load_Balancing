//// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "client_socket.h"
#include "message_validator.h"

int main() {
    ClientSocket client;

    //Initialize WSA
    if (!initialize_socket(&client)) {                                      //From: client_socket.h
        fprintf(stderr, "Failed to initialize client.\n");
        return EXIT_FAILURE;    //if false cleanup_socket is called inside initialize_socket
    }

    //PORT: 5059
    if (!connect_to_server(&client, "127.0.0.1", 5059)) {                   //From: client_socket.h
        fprintf(stderr, "Failed to connect to server.\n");
        return EXIT_FAILURE;    //if false cleanup_socket is called inside connect_to_server
    }

    //Recv: You are connected as CLIENT
    char response[255];
    if (!receive_message(&client, response, sizeof(response))) {            //From: client_socket.h
        fprintf(stderr, "Failed to receive message from server.\n");
    }

    printf("[Load Balancer]: %s\n", response);

    printf("-----Send Messages-----\n");
    char buffer[255] = { 0 };

    while (1) {
        printf("Enter message or type 'end' to exit: ");
        fgets(buffer, sizeof(buffer), stdin);

        // Remove trailing newline character
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "end") == 0) {
            printf("END\n");
            break;
        }

        if (strlen(buffer) == 0 || !is_valid_message(buffer)) {             //From: message_validator.h
            printf("Incorrect message content. The message was not sent. Try again.\n");
            continue;
        }
            
        if (!send_message(&client, buffer)) {                               //From: client_socket.h
            fprintf(stderr, "Failed to send message to server.\n");
            break;
        }

        char response[255];
        if (!receive_message(&client, response, sizeof(response))) {        //From: client_socket.h
            fprintf(stderr, "Failed to receive message from server.\n");
            break;
        }

        printf("[Load Balancer]: %s\n", response);
    }

    cleanup_socket(&client);                                                //From: client_socket.h
    return EXIT_SUCCESS;
}