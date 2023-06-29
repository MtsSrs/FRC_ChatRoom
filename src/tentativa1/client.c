#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

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

    // Criar o socket do cliente
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        perror("Erro ao criar o socket do cliente");
        exit(EXIT_FAILURE);
    }

    // Configurar o endereço do servidor
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(argv[1]);
    serverAddress.sin_port = htons(atoi(serverPort));

    if (inet_pton(AF_INET, serverIP, &(serverAddress.sin_addr)) <= 0)
    {
        perror("Erro ao configurar o endereço do servidor");
        exit(EXIT_FAILURE);
    }

    // Conectar ao servidor
    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("Erro ao conectar ao servidor");
        exit(EXIT_FAILURE);
    }

    // Enviar o nome de usuário e número da sala para o servidor
    sprintf(buffer, "%s:%d", userName, roomNumber);
    if (send(clientSocket, buffer, strlen(buffer), 0) < 0)
    {
        perror("Erro ao enviar informações para o servidor");
        exit(EXIT_FAILURE);
    }

    // Realizar o processamento do cliente
    while (1)
    {
        // Ler a entrada do usuário
        printf("Digite uma mensagem: ");
        fgets(buffer, BUFFER_SIZE, stdin);

        // Enviar a mensagem para o servidor
        if (send(clientSocket, buffer, strlen(buffer), 0) < 0)
        {
            perror("Erro ao enviar a mensagem para o servidor");
            exit(EXIT_FAILURE);
        }
    }

    // Fechar o socket do cliente
    close(clientSocket);

    return 0;
}
