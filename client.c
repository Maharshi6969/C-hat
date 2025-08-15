#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>   // for CreateThread
#include "helperfunctions.h"

#pragma comment(lib, "ws2_32.lib")

DWORD WINAPI recv_handler(LPVOID arg) {
    SOCKET sock = *(SOCKET*)arg;
    char buffer[1024];

    while (1) {
        int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            printf("\nServer disconnected.\n");
            exit(0);
        }
        buffer[bytes] = '\0';
        printf("\n%s\nYou: ", buffer); // reprint prompt after new message
        fflush(stdout);
    }
    return 0;
}

int main() {
    initWinsock();

    SOCKET client = createSocket();
    struct sockaddr_in server = makeAddress("127.0.0.1", 8080);

    connectSocket(client, server);
    if (client == INVALID_SOCKET) {
        printf("Failed to connect.\n");
        return 1;
    }
    printf("Connected to server.\n");

    // Wait for server's prompt and print it
    char prompt[128];
    int bytes = recv(client, prompt, sizeof(prompt) - 1, 0);
    if (bytes > 0) {
        prompt[bytes] = '\0';
        printf("%s", prompt);
    }

    // username
    char name[100];
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';
    send(client, name, (int)strlen(name), 0);

    // launch thread for receiving
    HANDLE recvThread = CreateThread(NULL, 0, recv_handler, &client, 0, NULL);
    if (recvThread == NULL) {
        printf("Failed to create receive thread.\n");
        closesocket(client);
        WSACleanup();
        return 1;
    }

    // main thread: send loop
    char buffer[1024];
    while (1) {
        printf("You: ");
        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strlen(buffer) == 0) continue;
        send(client, buffer, (int)strlen(buffer), 0);
    }

    closesocket(client);
    WSACleanup();
    return 0;
}
