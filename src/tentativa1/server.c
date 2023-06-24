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

#define MAX_CLIENTS 10
#define BUFFER_SIZE 2048

volatile sig_atomic_t flag = 0;
int sockfd = 0;
int client_sockets[MAX_CLIENTS];
char client_names[MAX_CLIENTS][32];
char client_rooms[MAX_CLIENTS][10];

void handleExit(int sig)
{
    flag = 1;
}

void *handleMsg(void *arg)
{
    int clientfd = *((int *)arg);
    char buffer[BUFFER_SIZE];

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        int receive = recv(clientfd, buffer, BUFFER_SIZE, 0);
        if (receive > 0)
        {
            printf("%s - Sala %s: %s\n", client_names[clientfd], client_rooms[clientfd], buffer);
        }
        else if (receive == 0)
        {
            printf("Client %s - Sala %s has left the chat\n", client_names[clientfd], client_rooms[clientfd]);
            break;
        }
        else
        {
            // -1
        }
    }

    close(clientfd);
    client_sockets[clientfd] = 0;
    strcpy(client_names[clientfd], "");
    strcpy(client_rooms[clientfd], "");

    return NULL;
}

int main(int argc, char **argv)
{
    char *ip = argv[1];
    char *port = argv[2];
    struct sockaddr_in serverAddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ip);
    serverAddr.sin_port = htons(atoi(port));

    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("ERROR: bind");
        return EXIT_FAILURE;
    }

    if (listen(sockfd, MAX_CLIENTS) < 0)
    {
        perror("ERROR: listen");
        return EXIT_FAILURE;
    }

    printf("=== WELCOME TO THE CHATROOM ===\n");

    fd_set read_fds;
    int max_socket_fd = sockfd;
    int i;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        client_sockets[i] = 0;
        strcpy(client_names[i], "");
        strcpy(client_rooms[i], "");
    }

    while (1)
    {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        max_socket_fd = sockfd;

        // Na primeira iteração do while esse loop não faz nada
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            int clientfd = client_sockets[i];
            if (clientfd > 0)
            {
                FD_SET(clientfd, &read_fds);
                if (clientfd > max_socket_fd)
                {
                    max_socket_fd = clientfd;
                }
            }
        }

        if (select(max_socket_fd + 1, &read_fds, NULL, NULL, NULL) < 0)
        {
            perror("ERROR: select");
            return EXIT_FAILURE;
        }

        if (FD_ISSET(sockfd, &read_fds))
        {
            struct sockaddr_in clientAddr;
            socklen_t client_addr_len = sizeof(clientAddr);
            int clientfd = accept(sockfd, (struct sockaddr *)&clientAddr, &client_addr_len);

            if (clientfd < 0)
            {
                perror("ERROR: accept");
                return EXIT_FAILURE;
            }

            for (i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_sockets[i] == 0)
                {
                    client_sockets[i] = clientfd;
                    printf("New client connected: %s\n", inet_ntoa(clientAddr.sin_addr));
                    break;
                }
            }
        }

        for (i = 0; i < MAX_CLIENTS; i++)
        {
            int clientfd = client_sockets[i];
            if (FD_ISSET(clientfd, &read_fds))
            {
                pthread_t receive_msg_thread;
                if (pthread_create(&receive_msg_thread, NULL, handleMsg, &clientfd) != 0)
                {
                    perror("ERROR: pthread");
                    return EXIT_FAILURE;
                }
            }
        }

        if (flag)
        {
            printf("\nServer stopped\n");
            break;
        }
    }

    for (i = 0; i < MAX_CLIENTS; i++)
    {
        if (client_sockets[i] != 0)
        {
            close(client_sockets[i]);
        }
    }

    close(sockfd);

    return EXIT_SUCCESS;
}
