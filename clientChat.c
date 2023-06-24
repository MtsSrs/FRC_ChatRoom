#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_MESSAGE_LEN 256

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s <IP> <porta>\n", argv[0]);
        exit(1);
    }

    int client_socket;
    struct sockaddr_in server_address;

    // Criação do socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Erro ao criar o socket");
        exit(1);
    }

    // Configuração do endereço do servidor
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(argv[1]);
    server_address.sin_port = htons(atoi(argv[2]));

    // Conexão com o servidor
    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Erro ao conectar ao servidor");
        exit(1);
    }

    // Recebendo mensagem de boas-vindas do servidor
    char welcome_message[MAX_MESSAGE_LEN];
    ssize_t bytes_received = recv(client_socket, welcome_message, MAX_MESSAGE_LEN, 0);
    if (bytes_received == -1) {
        perror("Erro ao receber mensagem de boas-vindas");
        close(client_socket);
        exit(1);
    }
    printf("%.*s\n", (int)bytes_received, welcome_message);

    // Lendo o nome do cliente
    char name[MAX_MESSAGE_LEN];
    printf("Digite seu nome: ");
    fgets(name, MAX_MESSAGE_LEN, stdin);

    // Enviando o nome do cliente para o servidor
    send(client_socket, name, strlen(name), 0);

    // Processamento das mensagens do cliente
    while (1) {
        char message[MAX_MESSAGE_LEN];

        // Lendo a mensagem do cliente
        printf("> ");
        fgets(message, MAX_MESSAGE_LEN, stdin);

        // Enviando a mensagem para o servidor
        send(client_socket, message, strlen(message), 0);
    }

    // Fechando o socket do cliente
    close(client_socket);

    return 0;
}
