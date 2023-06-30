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
    char userName[30];
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

        if (strcmp(buffer, "/list") == 0)
        {
            printf("%s", buffer);
        }
        else
        {
            printf("> %s\n", buffer);
        }

        fflush(stdout);

        if (strcmp(buffer, "Bem-vindo, ! Você está na sala \n") == 0)
        {
            printf("A mensagem de boas-vindas está vazia.\n");
        }
    }

    return NULL;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Uso: %s <IP do servidor> <Porta do servidor>\n", argv[0]);
        return 1;
    }

    int clientSocket;
    struct sockaddr_in serverAddress;
    char *serverIP = argv[1];
    char *serverPort = argv[2];

    char userName[30];
    int roomNumber = -1;

    char buffer[BUFFER_SIZE];

    printf("Digite seu Nome:\n");
    fgets(userName, sizeof(userName), stdin);

    // Remover o caractere de quebra de linha
    size_t userNameLength = strlen(userName);
    if (userName[userNameLength - 1] == '\n')
    {
        userName[userNameLength - 1] = '\0';
    }

    while (1)
    {
        printf("Digite o número da sala (0-9) ou digite /sair para sair:\n");
        char roomInput[10];
        fgets(roomInput, sizeof(roomInput), stdin);

        if (strcmp(roomInput, "/sair\n") == 0)
        {
            roomNumber = -1;
            printf("Você saiu da sala.\n");
            break;
        }

        roomNumber = atoi(roomInput);

        if (roomNumber < 0 || roomNumber > 9)
        {
            printf("Número de sala inválido.\n");
        }
        else
        {
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
            strcpy(threadData.userName, userName);

            pthread_t receiveThreadID;
            if (pthread_create(&receiveThreadID, NULL, receiveThread, (void *)&threadData) != 0)
            {
                perror("Erro ao criar a thread de recebimento\n");
                exit(EXIT_FAILURE);
            }

            while (1)
            {
                fgets(buffer, BUFFER_SIZE, stdin);

                size_t messageLength = strlen(buffer);
                if (messageLength > 1)
                {
                    buffer[messageLength - 1] = '\0';

                    if (strcmp(buffer, "/sair") == 0)
                    {
                        roomNumber = -1;
                        printf("Você saiu da sala.\n");
                        break;
                    }
                    else
                    {
                        if (send(clientSocket, buffer, strlen(buffer), 0) < 0)
                        {
                            perror("Erro ao enviar a mensagem para o servidor\n");
                            exit(EXIT_FAILURE);
                        }
                    }
                }
            }

            close(clientSocket);

            pthread_cancel(receiveThreadID);
            pthread_join(receiveThreadID, NULL);
        }
    }

    return 0;
}
