#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_CLIENTS 10
#define MAX_MESSAGE_LEN 256

typedef struct {
    int socket;
    char name[MAX_MESSAGE_LEN];
    int room;
} Client;

void handleClientMessage(Client* clients, int sender, char* message) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (i != sender && clients[i].socket != -1 && clients[i].room == clients[sender].room) {
            char formatted_message[MAX_MESSAGE_LEN];
            snprintf(formatted_message, MAX_MESSAGE_LEN, "%s: %s\n", clients[sender].name, message);
            send(clients[i].socket, formatted_message, strlen(formatted_message), 0);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <porta>\n", argv[0]);
        exit(1);
    }

    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_length;

    Client clients[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].socket = -1;
    }

    // Criação do socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Erro ao criar o socket");
        exit(1);
    }

    // Configuração do endereço do servidor
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(atoi(argv[1]));

    // Vinculação do socket ao endereço do servidor
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Erro ao vincular o socket ao endereço do servidor");
        exit(1);
    }

    // Aguardando conexões de clientes
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("Erro ao aguardar conexões de clientes");
        exit(1);
    }

    printf("Servidor de chat iniciado. Aguardando conexões...\n");

    while (1) {
        // Aceitando conexão de um novo cliente
        client_address_length = sizeof(client_address);
        client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_length);
        if (client_socket == -1) {
            perror("Erro ao aceitar conexão de cliente");
            continue;
        }

        // Encontrando um slot disponível para o novo cliente
        int new_client_index = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket == -1) {
                new_client_index = i;
                break;
            }
        }

        if (new_client_index == -1) {
            // Não há slots disponíveis, rejeita a conexão do novo cliente
            printf("Número máximo de clientes atingido. Conexão rejeitada.\n");
            close(client_socket);
            continue;
        }

        // Armazenando informações do novo cliente
        clients[new_client_index].socket = client_socket;
        clients[new_client_index].room = 0; // Sala padrão

        // Recebendo o nome do cliente
        ssize_t bytes_received = recv(client_socket, clients[new_client_index].name, MAX_MESSAGE_LEN, 0);
        if (bytes_received <= 0) {
            perror("Erro ao receber nome do cliente");
            close(client_socket);
            continue;
        }
        clients[new_client_index].name[bytes_received - 1] = '\0'; // Remove o caractere de nova linha

        printf("Novo cliente conectado: %s\n", clients[new_client_index].name);

        // Enviando mensagem de boas-vindas para o novo cliente
        char welcome_message[MAX_MESSAGE_LEN];
        snprintf(welcome_message, MAX_MESSAGE_LEN, "Bem-vindo, %s! Você está na sala %d.\n", clients[new_client_index].name, clients[new_client_index].room);
        send(client_socket, welcome_message, strlen(welcome_message), 0);

        // Notificando os clientes existentes sobre o novo cliente
        char notification[MAX_MESSAGE_LEN];
        snprintf(notification, MAX_MESSAGE_LEN, "O cliente %s entrou na sala %d.\n", clients[new_client_index].name, clients[new_client_index].room);
        handleClientMessage(clients, new_client_index, notification);

        // Processamento das mensagens do cliente
        while (1) {
            char message[MAX_MESSAGE_LEN];

            // Recebendo mensagem do cliente
            ssize_t bytes_received = recv(client_socket, message, MAX_MESSAGE_LEN, 0);
            if (bytes_received <= 0) {
                // Erro ou o cliente encerrou a conexão
                printf("Cliente %s desconectado.\n", clients[new_client_index].name);
                clients[new_client_index].socket = -1;
                snprintf(notification, MAX_MESSAGE_LEN, "O cliente %s saiu da sala %d.\n", clients[new_client_index].name, clients[new_client_index].room);
                handleClientMessage(clients, new_client_index, notification);
                break;
            }

            // Adiciona terminador de string à mensagem recebida
            message[bytes_received] = '\0';

            // Tratamento especial para comandos
            if (strncmp(message, "/sala", 5) == 0) {
                int new_room = atoi(&message[6]);
                if (new_room >= 0 && new_room <= 4) {
                    clients[new_client_index].room = new_room;
                    snprintf(notification, MAX_MESSAGE_LEN, "O cliente %s mudou para a sala %d.\n", clients[new_client_index].name, clients[new_client_index].room);
                    handleClientMessage(clients, new_client_index, notification);
                    continue;
                }
            }

            // Processando mensagem do cliente
            printf("Mensagem recebida de %s: %s\n", clients[new_client_index].name, message);
            handleClientMessage(clients, new_client_index, message);
        }
    }

    // Fechando sockets dos clientes
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket != -1) {
            close(clients[i].socket);
        }
    }

    // Fechando socket do servidor
    close(server_socket);

    return 0;
}
    