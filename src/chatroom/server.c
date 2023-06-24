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

typedef struct {
    char name[MAX_NAME_LENGTH];
    int room;
    int socket; // Adicionado membro 'socket'
} ClientSocket; // Alterado o nome da estrutura para 'ClientSocket'

void parseProtocolBuffer(char *protocol_buffer, char *userName, int *room) {
    sscanf(protocol_buffer, "%[^:]: %x", userName, room);
}

void sendToRoom(ClientSocket *clients, int numClients, int room, char *message) {
    for (int i = 0; i < numClients; i++) {
        if (clients[i].room == room) {
            send(clients[i].socket, message, strlen(message), 0);
        }
    }
}

int main(int argc, char **argv) {
    int sockfd = 0;
    int clientSockfd = 0;
    char protocol_buffer[BUFFER_SIZE];
    ClientSocket clients[MAX_CLIENTS]; // Alterado o tipo de dados para 'ClientSocket'
    int numClients = 0;

    char *ip = argv[1];
    char *port = argv[2];

    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Erro ao criar o socket\n");
        return EXIT_FAILURE;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ip);
    serverAddr.sin_port = htons(atoi(port));

    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Erro ao fazer o bind\n");
        return EXIT_FAILURE;
    }

    if (listen(sockfd, MAX_CLIENTS) < 0) {
        printf("Erro ao escutar a porta\n");
        return EXIT_FAILURE;
    }

    printf("Aguardando conexões...\n");

    while (1) {
        clientAddrLen = sizeof(clientAddr);
        clientSockfd = accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen);

        if (clientSockfd < 0) {
            printf("Erro ao aceitar a conexão\n");
            return EXIT_FAILURE;
        }

        printf("Conexão estabelecida\n");

        ssize_t bytesRead = recv(clientSockfd, protocol_buffer, BUFFER_SIZE - 1, 0);
        if (bytesRead < 0) {
            printf("Erro ao receber dados\n");
            return EXIT_FAILURE;
        }

        protocol_buffer[bytesRead] = '\0';
        parseProtocolBuffer(protocol_buffer, clients[numClients].name, &(clients[numClients].room));

        printf("%s entrou na sala %d\n", clients[numClients].name, clients[numClients].room);

        // Armazena o socket do cliente
        clients[numClients].socket = clientSockfd;

        numClients++;

        if (numClients >= MAX_CLIENTS) {
            printf("Número máximo de clientes atingido. Encerrando conexões novas.\n");
            break;
        }
    }

    while (1) {
        char roomStr[10];
        int room;
        char message[BUFFER_SIZE];

        printf("\nDigite o número da sala: ");
        fgets(roomStr, sizeof(roomStr), stdin);
        room = atoi(roomStr);

        printf("Digite a mensagem: ");
        fgets(message, sizeof(message), stdin);

        char completeMessage[BUFFER_SIZE];
        snprintf(completeMessage, sizeof(completeMessage), "%s: %s", clients[numClients - 1].name, message);

        sendToRoom(clients, numClients, room, completeMessage);
    }

    close(sockfd);

    return 0;
}
