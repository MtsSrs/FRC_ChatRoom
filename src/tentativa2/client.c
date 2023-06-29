#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

typedef struct
{
    int socket;
} ReceiveThreadData;

void *receiveThread(void *arg)
{
    ReceiveThreadData *threadData = (ReceiveThreadData *)arg;
    int clientSocket = threadData->socket;
    char buffer[BUFFER_SIZE];

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));

        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead <= 0)
            break;

        printf("Mensagem recebida do servidor: %s\n", buffer);
    }

    return NULL;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Uso: %s <IP do servidor> <Porta do servidor> <Nome do usuário> <Número da sala>\n", argv[0]);
        return 1;
    }

    int clientSocket;
    struct sockaddr_in serverAddress;
    char *serverIP = argv[1];
    char *serverPort = argv[2];

    char userName[30];
    int roomNumber;

    char buffer[BUFFER_SIZE];

    printf("Digite seu Nome:\n");
    fgets(userName, 30, stdin);
    printf("Digite o número da Sala:\n");
    scanf("%d", &roomNumber);

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        perror("Erro ao criar o socket do cliente\n");
        exit(EXIT_FAILURE);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(argv[1]);
    serverAddress.sin_port = htons(atoi(serverPort));

    if (inet_pton(AF_INET, serverIP, &(serverAddress.sin_addr)) <= 0)
    {
        perror("Erro ao configurar o endereço do servidor\n");
        exit(EXIT_FAILURE);
    }

    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("Erro ao conectar ao servidor\n");
        exit(EXIT_FAILURE);
    }

    sprintf(buffer, "%s:%d", userName, roomNumber);
    if (send(clientSocket, buffer, strlen(buffer), 0) < 0)
    {
        perror("Erro ao enviar informações para o servidor\n");
        exit(EXIT_FAILURE);
    }

    ReceiveThreadData threadData;
    threadData.socket = clientSocket;
    pthread_t receiveThreadID;
    if (pthread_create(&receiveThreadID, NULL, receiveThread, (void *)&threadData) != 0)
    {
        perror("Erro ao criar a thread de recebimento\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        printf("Digite uma mensagem: \n");
        fgets(buffer, BUFFER_SIZE, stdin);

        size_t messageLength = strlen(buffer);
        if (buffer[messageLength - 1] == '\n')
            buffer[messageLength - 1] = '\0';

        if (send(clientSocket, buffer, strlen(buffer), 0) < 0)
        {
            perror("Erro ao enviar a mensagem para o servidor\n");
            exit(EXIT_FAILURE);
        }
    }

    close(clientSocket);

    return 0;
}
