#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct
{
    int clientSocket;
    int roomNumber;
} ClientInfo;

void handleClientMessage(ClientInfo *clients, int numClients, int clientIndex, char *message)
{
    // Percorrer todos os clientes na mesma sala e enviar a mensagem
    for (int i = 0; i < numClients; i++)
    {
        if (clients[i].roomNumber == clients[clientIndex].roomNumber && i != clientIndex)
        {
            if (send(clients[i].clientSocket, message, strlen(message), 0) < 0)
            {
                perror("Erro ao enviar mensagem para cliente");
                exit(EXIT_FAILURE);
            }
        }
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Uso: %s <Porta do servidor>\n", argv[0]);
        return 1;
    }

    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;
    char *serverPort = argv[2];
    char buffer[BUFFER_SIZE];
    ClientInfo clients[MAX_CLIENTS];
    int numClients = 0;

    // Criar o socket do servidor
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        perror("Erro ao criar o socket do servidor");
        exit(EXIT_FAILURE);
    }

    // Configurar o endereço do servidor
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(argv[1]);
    serverAddress.sin_port = htons(atoi(serverPort));

    // serverAddr.sin_port = htons(port);
    // serverAddr.sin_addr.s_addr = inet_addr(ip);

    // Associar o socket do servidor ao endereço e à porta
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("Erro ao associar o socket do servidor ao endereço e à porta");
        exit(EXIT_FAILURE);
    }

    // Colocar o socket em modo de escuta
    if (listen(serverSocket, 5) < 0)
    {
        perror("Erro ao colocar o socket em modo de escuta");
        exit(EXIT_FAILURE);
    }

    printf("Servidor escutando na porta %s...\n", serverPort);

    // Aceitar conexões de clientes
    while (1)
    {
        socklen_t addrlen = sizeof(clientAddress);
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &addrlen);
        if (clientSocket < 0)
        {
            perror("Erro ao aceitar conexão de cliente");
            exit(EXIT_FAILURE);
        }

        // Receber as informações do cliente
        memset(buffer, 0, sizeof(buffer));
        if (recv(clientSocket, buffer, sizeof(buffer), 0) < 0)
        {
            perror("Erro ao receber informações do cliente");
            exit(EXIT_FAILURE);
        }

        // Extrair o nome de usuário e número da sala
        char *userName = strtok(buffer, ":");
        int roomNumber = atoi(strtok(NULL, ":"));

        printf("Novo cliente conectado - Usuário: %s, Sala: %d\n", userName, roomNumber);

        // Adicionar o cliente à lista de clientes
        clients[numClients].clientSocket = clientSocket;
        clients[numClients].roomNumber = roomNumber;
        numClients++;

        // Enviar mensagem de boas-vindas ao cliente
        sprintf(buffer, "Bem-vindo, %s! Você está na sala %d\n", userName, roomNumber);
        if (send(clientSocket, buffer, strlen(buffer), 0) < 0)
        {
            perror("Erro ao enviar mensagem de boas-vindas ao cliente");
            exit(EXIT_FAILURE);
        }

        // Criar uma nova thread para tratar as mensagens do cliente
        // (opcional: você pode implementar isso usando pthreads ou outras bibliotecas de threads)
    }

    // Fechar o socket do servidor
    close(serverSocket);

    return 0;
}
