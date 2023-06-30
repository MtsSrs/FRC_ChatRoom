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

intToHex(int num, char *hex)
{
    sprintf(hex, "%x", num);
}

int main(int argc, char **argv)
{
    int sockfd = 0;
    char protocol_buffer[BUFFER_SIZE];
    char *ip = argv[1];
    char *port = argv[2];
    char *userName[32];
    int room;

    printf("Digite seu nome:\n");
    fgets(userName, sizeof(userName), stdin);

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

    while (1)
    {
        break;
    }

    return 0;
}