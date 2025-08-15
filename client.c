#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string.h>
#include <stdlib.h>
#include "helperfunctions.h"

#pragma comment(lib, "ws2_32.lib")

// Forward declarations
SOCKET init_client();
DWORD WINAPI recv_handler(LPVOID arg);

int main() {
    SOCKET clientSocket = init_client();

    // Receive "Enter your name: " prompt from server
    char buffer[1024];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
        printf("Failed to receive welcome message.\n");
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
    buffer[bytesReceived] = '\0';
    printf("%s", buffer);

    // Send username
    char name[100];
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';
    send(clientSocket, name, (int)strlen(name), 0);

    // Start thread to receive messages
    HANDLE recvThread = CreateThread(NULL, 0, recv_handler, &clientSocket, 0, NULL);
    if (recvThread == NULL) {
        printf("Failed to create receive thread.\n");
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
    CloseHandle(recvThread); // We don't need to wait for it

    // Main thread: send messages
    while (1) {
        printf("You: ");
        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        buffer[strcspn(buffer, "\n")] = '\0';
        if (strlen(buffer) == 0) continue;

        int sent = send(clientSocket, buffer, (int)strlen(buffer), 0);
        if (sent == SOCKET_ERROR) {
            printf("Send failed, disconnecting.\n");
            break;
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}

SOCKET init_client() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed.\n");
        exit(1);
    }

    char hostname[256];
    char port[10];
    printf("Enter server hostname (e.g. serveo.net): ");
    scanf("%255s", hostname);
    printf("Enter server port: ");
    scanf("%9s", port);

    struct addrinfo hints, *res;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(hostname, port, &hints, &res) != 0) {
        printf("DNS resolution failed.\n");
        WSACleanup();
        exit(1);
    }

    SOCKET clientSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (clientSocket == INVALID_SOCKET) {
        printf("Socket creation failed.\n");
        freeaddrinfo(res);
        WSACleanup();
        exit(1);
    }

    if (connect(clientSocket, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR) {
        printf("Connect failed.\n");
        closesocket(clientSocket);
        freeaddrinfo(res);
        WSACleanup();
        exit(1);
    }

    freeaddrinfo(res);
    printf("Connected to server.\n");
    return clientSocket;
}


DWORD WINAPI recv_handler(LPVOID arg) {
    SOCKET sock = *(SOCKET*)arg;
    char buffer[1024];

    while (1) {
        int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            printf("\nDisconnected from server.\n");
            exit(0);
        }
        buffer[bytes] = '\0';
        printf("\n%s\nYou: ", buffer);
        fflush(stdout);
    }
    return 0;
}
