#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 2048

int intToHex(int num, char *hex)
{
    sprintf(hex, "%x", num);
}

int main(int argc, char **argv)
{
    int sockfd = 0;
    char protocol_buffer[BUFFER_SIZE];
    char *ip = argv[1]; // Alteração: Armazenar o endereço IP em um array fixo
    char *port = argv[2];
    char userName[32]; // Alteração: Corrigir declaração do array userName
    int room;

    printf("Digite seu nome:\n");
    fgets(userName, sizeof(userName), stdin);

    // printf("Digite o endereço IP do servidor:\n"); // Alteração: Solicitar o endereço IP
    // fgets(ip, sizeof(ip), stdin);

    printf("Digite a sala escolhida:\n");
    scanf("%d", &room);

    char hexRoom[10];
    intToHex(room, hexRoom);

    sprintf(protocol_buffer, "%s: %s\n", userName, hexRoom);

    struct sockaddr_in serverAddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ip);
    serverAddr.sin_port = htons(atoi(port));

    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        printf("Erro na conexão\n");
        return EXIT_FAILURE;
    }

    send(sockfd, protocol_buffer, strlen(protocol_buffer), 0);

    printf("Conexão estabelecida. Agora você pode enviar mensagens para o servidor.\n");

    while (1)
    {
        char message[BUFFER_SIZE];
        fgets(message, sizeof(message), stdin);

        // Remover o caractere '\n' da string de entrada
        size_t len = strlen(message);
        if (len > 0 && message[len - 1] == '\n')
        {
            message[len - 1] = '\0';
        }

        send(sockfd, message, strlen(message), 0);

        ssize_t bytesReceived = recv(sockfd, protocol_buffer, BUFFER_SIZE - 1, 0);

        if (bytesReceived < 0)
        {
            printf("Erro ao receber dados do servidor\n");
            return EXIT_FAILURE;
        }
        else if (bytesReceived == 0)
        {
            printf("Conexão encerrada pelo servidor\n");
            break;
        }
        else
        {
            protocol_buffer[bytesReceived] = '\0';
            printf("[Servidor] Mensagem recebida: %s\n", protocol_buffer);
        }

        // Se a mensagem for "exit", encerrar o loop
        if (strcmp(message, "exit") == 0)
        {
            break;
        }
    }

    close(sockfd);

    return 0;
}
