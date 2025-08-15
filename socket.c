#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string.h>
#include <pthread.h>
#include "helperfunctions.h"
#define MAX_CLIENTS 100
struct client_info {
    SOCKET sock;
    char name[32];
};

struct client_array {
    struct client_info clients[MAX_CLIENTS];
    int count;
    pthread_mutex_t lock;
};


//to send both the necessary things into the thread arg as it accepts only one argument...
struct thread_args{
    SOCKET clientSock;
    struct client_array* clientsList;
};
#pragma comment(lib, "ws2_32.lib")

SOCKET init_server();
void add_client(SOCKET client, struct client_array* clients);
void remove_client(SOCKET client, struct client_array* clients);
void* client_handler(void* arg);

int main() {
   SOCKET server = init_server();
    struct client_array clients;
    pthread_mutex_init(&clients.lock,NULL);
    clients.count = 0;
    while(1){
        SOCKET client = acceptClient(server);
        struct thread_args *args = malloc(sizeof(struct thread_args));
        if (args == NULL) {
            closesocket(client);
            continue;
        }

        args->clientSock = client;
        args->clientsList = &clients;

        add_client(client,&clients);

        pthread_t tid;
        if (pthread_create(&tid, NULL, client_handler, (void*)args) != 0) {
            remove_client(client,&clients);
            free(args);
            closesocket(client);
            continue;
        }

        pthread_detach(tid);
    }

}

SOCKET init_server(){
    initWinsock();
    SOCKET serverSocket = createSocket();
    struct sockaddr_in serverAddr = makeAddress("127.0.0.1",8080);
    bindSocket(serverSocket, 8080);
    listenSocket(serverSocket, 5);
    printf("Server listening on port 8080...\n");
    return serverSocket;
}

void add_client(SOCKET client, struct client_array* clients) {
    pthread_mutex_lock(&clients->lock);
    if (clients->count < MAX_CLIENTS) {
        clients->clients[clients->count].sock = client;
        // Initialize name as empty string for now; will fill after receiving it
        clients->clients[clients->count].name[0] = '\0';
        clients->count++;
        pthread_mutex_unlock(&clients->lock);
    } else {
        pthread_mutex_unlock(&clients->lock);
        closesocket(client); // reject new connection politely
    }
}


void remove_client(SOCKET client, struct client_array* clients) {
    pthread_mutex_lock(&clients->lock);
    int found = -1;
    for (int i = 0; i < clients->count; i++) {
        if (clients->clients[i].sock == client) {
            found = i;
            break;
        }
    }
    if (found != -1) {
        for (int j = found; j < clients->count - 1; j++) {
            clients->clients[j] = clients->clients[j + 1];  // struct assignment copies both sock and name
        }
        clients->count--;
    }
    pthread_mutex_unlock(&clients->lock);
}


void* client_handler(void* arg) {
    struct thread_args *args = (struct thread_args*)arg;
    SOCKET clientSock = args->clientSock;
    struct client_array *clients = args->clientsList;
    free(args);

    const char *askName = "Enter your name: ";
    send(clientSock, askName, strlen(askName), 0);

    char name[32];
    int bytesReceived = recvMessage(clientSock, name, sizeof(name));
    if (bytesReceived <= 0) {
        closesocket(clientSock);
        return NULL;
    }

    // Store the name in clients array
    pthread_mutex_lock(&clients->lock);
    int clientIndex = -1;
    for (int i = 0; i < clients->count; i++) {
        if (clients->clients[i].sock == clientSock) {
            clientIndex = i;
            strncpy(clients->clients[i].name, name, 31);
            clients->clients[i].name[31] = '\0'; // null-terminate
            break;
        }
    }
    pthread_mutex_unlock(&clients->lock);

    if (clientIndex == -1) {
        closesocket(clientSock);
        return NULL;
    }

    char buffer[1024];

    while (1) {
        int bytesReceived = recvMessage(clientSock, buffer, sizeof(buffer));
        if (bytesReceived <= 0) break;

        pthread_mutex_lock(&clients->lock);
        int count = clients->count;
        struct client_info copyClients[MAX_CLIENTS];
        memcpy(copyClients, clients->clients, sizeof(struct client_info) * count);
        pthread_mutex_unlock(&clients->lock);

        char messageWithName[1100];
        snprintf(messageWithName, sizeof(messageWithName), "%s: %s", copyClients[clientIndex].name, buffer);

        for (int i = 0; i < count; i++) {
            if (copyClients[i].sock != clientSock) {
                sendMessage(copyClients[i].sock, messageWithName);
            }
        }
    }

    remove_client(clientSock, clients);
    closesocket(clientSock);
    return NULL;
}
