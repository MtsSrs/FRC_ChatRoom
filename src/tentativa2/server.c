#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_CLIENTS 500
#define BUFFER_SIZE 1024

typedef struct
{
    int clientSocket;
    int roomNumber;
    char *userName;
} ClientInfo;

typedef struct
{
    int numClients;
    ClientInfo clients[MAX_CLIENTS];
} Room;

typedef struct
{
    Room *rooms;
    int roomNumber;
    int clientIndex;
} ThreadData;

void handleClientMessage(Room *rooms, int roomNumber, int clientIndex, char *message)
{
    char userMessage[BUFFER_SIZE];
    sprintf(userMessage, "%s: %s", rooms[roomNumber].clients[clientIndex].userName, message);

    for (int i = 0; i < rooms[roomNumber].numClients; i++)
    {
        int clientSocket = rooms[roomNumber].clients[i].clientSocket;

        if (send(clientSocket, userMessage, strlen(userMessage), 0) < 0)
        {
            perror("Erro ao enviar mensagem para cliente");
            exit(EXIT_FAILURE);
        }
    }
}

void showUsers(Room *rooms, int roomNumber, int clientIndex)
{
    int clientSocket = rooms[roomNumber].clients[clientIndex].clientSocket;
    for (int i = 0; i < rooms[roomNumber].numClients; i++)
    {
        char userName[30];
        strcpy(userName, rooms[roomNumber].clients[i].userName);

        int userNameLength = strlen(userName) + 1;
        userName[userNameLength - 1] = '\n';

        if (send(clientSocket, userName, userNameLength, 0) < 0)
        {
            perror("Erro ao listar usuários para o cliente");
            exit(EXIT_FAILURE);
        }
    }
}

void *clientThread(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    Room *rooms = data->rooms;
    int roomNumber = data->roomNumber;
    int clientIndex = data->clientIndex;
    int clientSocket = rooms[roomNumber].clients[clientIndex].clientSocket;

    char buffer[BUFFER_SIZE];

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesRead <= 0)
        {
            perror("Erro ao receber mensagem do cliente");
            exit(EXIT_FAILURE);
        }
        else if (strcmp(buffer, "/list") == 0)
            showUsers(rooms, roomNumber, clientIndex);
        else
            handleClientMessage(rooms, roomNumber, clientIndex, buffer);
    }

    return NULL;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Uso: %s <Endereço IP> <Porta do servidor>\n", argv[0]);
        return 1;
    }

    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;
    char *serverIP = argv[1];
    char *serverPort = argv[2];
    char buffer[BUFFER_SIZE];
    Room rooms[MAX_CLIENTS];
    int numRooms = 0;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        perror("Erro ao criar o socket do servidor");
        exit(EXIT_FAILURE);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(serverIP);
    serverAddress.sin_port = htons(atoi(serverPort));

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("Erro ao associar o socket do servidor ao endereço e à porta");
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 5) < 0)
    {
        perror("Erro ao colocar o socket em modo de escuta");
        exit(EXIT_FAILURE);
    }

    printf("Servidor escutando na porta %s...\n", serverPort);

    while (1)
    {
        socklen_t addrlen = sizeof(clientAddress);
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &addrlen);
        if (clientSocket < 0)
        {
            perror("Erro ao aceitar conexão de cliente");
            exit(EXIT_FAILURE);
        }

        memset(buffer, 0, sizeof(buffer));
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead < 0)
        {
            perror("Erro ao receber informações do cliente");
            exit(EXIT_FAILURE);
        }

        char *userName = strtok(buffer, ":");
        int roomNumber = atoi(strtok(NULL, ":"));

        printf("Novo cliente conectado - Usuário: %s Sala: %d\n", userName, roomNumber);

        int clientIndex = rooms[roomNumber].numClients;
        rooms[roomNumber].clients[clientIndex].clientSocket = clientSocket;
        rooms[roomNumber].clients[clientIndex].roomNumber = roomNumber;
        rooms[roomNumber].clients[clientIndex].userName = strdup(userName); // Alocar e copiar o nome do usuário

        rooms[roomNumber].numClients++;

        char welcomeMessage[BUFFER_SIZE];
        sprintf(welcomeMessage, "Bem-vindo, %s! Você está na sala %d\n", userName, roomNumber);
        if (send(clientSocket, welcomeMessage, strlen(welcomeMessage), 0) < 0)
        {
            perror("Erro ao enviar mensagem de boas-vindas ao cliente");
            exit(EXIT_FAILURE);
        }

        char joinMessage[BUFFER_SIZE];
        sprintf(joinMessage, "Usuário %s entrou na sala\n", userName);
        handleClientMessage(rooms, roomNumber, clientIndex, joinMessage);

        pthread_t tid;
        ThreadData *data = malloc(sizeof(ThreadData));
        data->rooms = rooms;
        data->roomNumber = roomNumber;
        data->clientIndex = clientIndex;

        if (pthread_create(&tid, NULL, clientThread, data) != 0)
        {
            perror("Erro ao criar thread do cliente");
            exit(EXIT_FAILURE);
        }

        pthread_detach(tid);
    }

    close(serverSocket);

    return 0;
}
