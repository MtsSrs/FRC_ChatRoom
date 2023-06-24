#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 2048
#define MAX_CLIENTS 10
#define MAX_NAME_LENGTH 32

typedef struct
{
    char name[MAX_NAME_LENGTH];
    int room;
    int sockfd; // Novo campo para armazenar o descritor de arquivo do socket
} ClientData;

void parseProtocolBuffer(char *protocol_buffer, char *userName, int *room)
{
    sscanf(protocol_buffer, "%[^:]: %x", userName, room);
}

void sendMessageToRoomClients(ClientData *clients, int numClients, int room, char *message)
{
    for (int i = 0; i < numClients; i++)
    {
        if (clients[i].room == room)
        {
            send(clients[i].sockfd, message, strlen(message), 0);
        }
    }
}

int main(int argc, char **argv)
{
    int sockfd = 0;
    int clientSockfd = 0;
    char protocol_buffer[BUFFER_SIZE];
    ClientData clients[MAX_CLIENTS];
    int numClients = 0;

    char *ip = argv[1];
    char *port = argv[2];

    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("Erro ao criar o socket\n");
        return EXIT_FAILURE;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ip);
    serverAddr.sin_port = htons(atoi(port));

    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        printf("Erro ao fazer o bind\n");
        return EXIT_FAILURE;
    }

    if (listen(sockfd, MAX_CLIENTS) < 0)
    {
        printf("Erro ao escutar a porta\n");
        return EXIT_FAILURE;
    }

    printf("Aguardando conexões...\n");

    while (1)
    {
        clientAddrLen = sizeof(clientAddr);
        clientSockfd = accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen);

        if (clientSockfd < 0)
        {
            printf("Erro ao aceitar a conexão\n");
            return EXIT_FAILURE;
        }

        printf("Conexão estabelecida\n");

        ssize_t bytesRead = recv(clientSockfd, protocol_buffer, BUFFER_SIZE - 1, 0);
        if (bytesRead < 0)
        {
            printf("Erro ao receber dados\n");
            return EXIT_FAILURE;
        }

        protocol_buffer[bytesRead] = '\0';
        parseProtocolBuffer(protocol_buffer, clients[numClients].name, &(clients[numClients].room));

        printf("Nome: %s\n", clients[numClients].name);
        printf("Sala: %d\n", clients[numClients].room);

        clients[numClients].sockfd = clientSockfd; // Armazenar o descritor de arquivo do socket

        numClients++;

        if (numClients >= MAX_CLIENTS)
        {
            printf("Número máximo de clientes atingido. Encerrando conexões novas.\n");
            break;
        }

        // Enviar mensagem de boas-vindas para o cliente
        char welcomeMessage[BUFFER_SIZE];
        sprintf(welcomeMessage, "Bem-vindo, %s! Você está na sala %d\n", clients[numClients - 1].name, clients[numClients - 1].room);
        send(clientSockfd, welcomeMessage, strlen(welcomeMessage), 0);

        // Enviar mensagem de entrada para outros clientes na mesma sala
        char enterMessage[BUFFER_SIZE];
        sprintf(enterMessage, "%s entrou na sala.\n", clients[numClients - 1].name);
        sendMessageToRoomClients(clients, numClients, clients[numClients - 1].room, enterMessage);
    }

    printf("Conexões encerradas. Aguardando mensagens...\n");

    while (1)
    {
        for (int i = 0; i < numClients; i++)
        {
            clientSockfd = clients[i].sockfd;

            ssize_t bytesRead = recv(clientSockfd, protocol_buffer, BUFFER_SIZE - 1, 0);
            if (bytesRead < 0)
            {
                printf("Erro ao receber dados\n");
                return EXIT_FAILURE;
            }
            else if (bytesRead == 0)
            {
                // O cliente fechou a conexão
                printf("%s saiu da sala.\n", clients[i].name);

                // Enviar mensagem de saída para outros clientes na mesma sala
                char exitMessage[BUFFER_SIZE];
                sprintf(exitMessage, "%s saiu da sala.\n", clients[i].name);
                sendMessageToRoomClients(clients, numClients, clients[i].room, exitMessage);

                // Fechar o socket do cliente e remover do array de clientes
                close(clientSockfd);
                for (int j = i; j < numClients - 1; j++)
                {
                    clients[j] = clients[j + 1];
                }
                numClients--;
                i--;
            }
            else
            {
                protocol_buffer[bytesRead] = '\0';
                printf("[%s] Mensagem recebida: %s\n", clients[i].name, protocol_buffer);

                // Enviar mensagem para outros clientes na mesma sala
                char message[BUFFER_SIZE];
                sprintf(message, "[%s] %s\n", clients[i].name, protocol_buffer);
                sendMessageToRoomClients(clients, numClients, clients[i].room, message);
            }
        }
    }

    close(sockfd);

    return 0;
}
