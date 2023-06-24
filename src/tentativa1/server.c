#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_NAME_LENGTH 50
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct
{
    char name[MAX_NAME_LENGTH];
    int room;
    int sockfd;
} ClientData;

void sendMessageToRoomClients(ClientData *client, ClientData *clients, int numClients, const char *message)
{
    for (int i = 0; i < numClients; i++)
    {
        if (clients[i].room == client->room && clients[i].sockfd != client->sockfd)
        {
            send(clients[i].sockfd, message, strlen(message), 0);
        }
    }
}

void *threadFunction1(void *arg)
{
    int sockfd = *(int *)arg;
    char buffer[BUFFER_SIZE];
    ClientData clients[MAX_CLIENTS];
    int numClients = 0;

    while (1)
    {
        // Receber nome do cliente
        int bytesRead = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
        if (bytesRead <= 0)
        {
            // Erro ou conexão fechada pelo cliente
            break;
        }
        buffer[bytesRead] = '\0';

        // Adicionar cliente à lista
        strcpy(clients[numClients].name, buffer);
        clients[numClients].room = 1; // Definir sala fixa para exemplo
        clients[numClients].sockfd = sockfd;
        numClients++;

        // Enviar mensagem de entrada para os outros clientes
        char message[BUFFER_SIZE];
        snprintf(message, BUFFER_SIZE, "%s entrou na sala.\n", buffer);
        sendMessageToRoomClients(&clients[numClients - 1], clients, numClients, message);
    }

    // Cliente desconectado, remover da lista
    for (int i = 0; i < numClients; i++)
    {
        if (clients[i].sockfd == sockfd)
        {
            // Enviar mensagem de saída para os outros clientes
            char message[BUFFER_SIZE];
            snprintf(message, BUFFER_SIZE, "%s saiu da sala.\n", clients[i].name);
            sendMessageToRoomClients(&clients[i], clients, numClients, message);

            // Remover cliente da lista
            for (int j = i; j < numClients - 1; j++)
            {
                clients[j] = clients[j + 1];
            }
            numClients--;
            break;
        }
    }

    close(sockfd);
    return NULL;
}

void *threadFunction2(void *arg)
{
    ClientData *client = (ClientData *)arg;
    char buffer[BUFFER_SIZE];
    ClientData *clients = (ClientData *)arg;                       // Adicionar declaração de clients
    int numClients = *(int *)((char *)arg + sizeof(ClientData *)); // Adicionar declaração de numClients

    while (1)
    {
        int bytesRead = recv(client->sockfd, buffer, BUFFER_SIZE - 1, 0);
        if (bytesRead <= 0)
        {
            // Erro ou conexão fechada pelo cliente
            break;
        }
        buffer[bytesRead] = '\0';

        // Enviar mensagem para os outros clientes na sala
        char message[BUFFER_SIZE];
        snprintf(message, BUFFER_SIZE, "%s: %s", client->name, buffer);
        sendMessageToRoomClients(client, clients, numClients, message);
    }

    close(client->sockfd);
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Uso: %s [IP] [PORTA]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Erro ao criar socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip);

    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Erro ao fazer bind");
        return EXIT_FAILURE;
    }

    if (listen(sockfd, MAX_CLIENTS) < 0)
    {
        perror("Erro ao ouvir na porta");
        return EXIT_FAILURE;
    }

    printf("Servidor iniciado. Aguardando conexões...\n");

    while (1)
    {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSockfd = accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSockfd < 0)
        {
            perror("Erro ao aceitar conexão");
            return EXIT_FAILURE;
        }

        pthread_t thread;
        if (pthread_create(&thread, NULL, threadFunction1, &clientSockfd) != 0)
        {
            perror("Erro ao criar thread");
            return EXIT_FAILURE;
        }

        printf("Novo cliente conectado.\n");
    }

    close(sockfd);
    return EXIT_SUCCESS;
}
