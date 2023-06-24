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
} ClientData;

void parseProtocolBuffer(char *protocol_buffer, char *userName, int *room)
{
    sscanf(protocol_buffer, "%[^:]: %x", userName, room);
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

        numClients++;

        if (numClients >= MAX_CLIENTS)
        {
            printf("Número máximo de clientes atingido. Encerrando conexões novas.\n");
            break;
        }

        close(clientSockfd);
    }

    close(sockfd);

    // Exemplo de como acessar os dados armazenados na matriz
    printf("\nDados dos clientes:\n");
    for (int i = 0; i < numClients; i++)
    {
        printf("Cliente %d:\n", i + 1);
        printf("Nome: %s\n", clients[i].name);
        printf("Sala: %d\n", clients[i].room);
        printf("\n");
    }

    return 0;
}
