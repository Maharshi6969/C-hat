#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string.h>

// 1. Initialize Winsock
void initWinsock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (result != 0) {
        printf("WSAStartup failed: %d\n", result);
        exit(1);
    }
}

// 2. Create a socket
SOCKET createSocket() {
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        printf("Socket creation failed! Error: %d\n", WSAGetLastError());
        WSACleanup();
        exit(1);
    }
    return s;
}

// 3. Bind a socket
void bindSocket(SOCKET s, int port) {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("Bind failed! Error: %d\n", WSAGetLastError());
        closesocket(s);
        WSACleanup();
        exit(1);
    }
}

// 4. Listen for connections
void listenSocket(SOCKET s,int queueSize) {
    if (listen(s, queueSize) == SOCKET_ERROR) {
        printf("Listen failed! Error: %d\n", WSAGetLastError());
        closesocket(s);
        WSACleanup();
        exit(1);
    }
}

// 5. Accept a client
SOCKET acceptClient(SOCKET serverSocket) {
    SOCKET client = accept(serverSocket, NULL, NULL);
    if (client == INVALID_SOCKET) {
        printf("Accept failed! Error: %d\n", WSAGetLastError());
    }
    return client;
}

struct sockaddr_in makeAddress(const char *ip, int port) {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    return addr;
}

void connectSocket(SOCKET s, struct sockaddr_in addr) {
    if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("Connection failed! Error: %d\n", WSAGetLastError());
        closesocket(s);
        WSACleanup();
        exit(1);
    }
}

int recvMessage(SOCKET sock, char *buffer, int bufferSize) {
    int bytesReceived = recv(sock, buffer, bufferSize - 1, 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
    }
    return bytesReceived;
}

void sendMessage(SOCKET sock, const char *msg) {
    size_t len = strlen(msg);
    if (send(sock, msg, (int)len, 0) == SOCKET_ERROR) {
        printf("Send failed! Error code: %d\n", WSAGetLastError());
    }
}
