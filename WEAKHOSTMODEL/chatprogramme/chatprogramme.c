#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 10

// Structure to hold client information
typedef struct {
    int sockfd;
    pthread_t thread;
    int color;
    char ip_address[INET_ADDRSTRLEN];  // Store the client IP address
} client_t;

client_t *clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;

// ANSI color escape codes
const char *colors[] = {
    "\033[31m", // Red
    "\033[32m", // Green
    "\033[33m", // Yellow
    "\033[34m", // Blue
    "\033[35m", // Magenta
    "\033[36m", // Cyan
    "\033[37m", // White
};

// Function to send messages to all clients except the sender
void broadcast_input(const char *message, client_t *sender) {
    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i] != NULL && clients[i] != sender) {
            char colored_message[2048];
            snprintf(colored_message, sizeof(colored_message), "%sClient %d (%s): %s\033[0m", 
                     colors[sender->color], sender->sockfd, sender->ip_address, message);
            send(clients[i]->sockfd, colored_message, strlen(colored_message), 0);
        }
    }
    pthread_mutex_unlock(&client_mutex);
}

// Function to handle client communication
void *handle_client(void *arg) {
    client_t *client = (client_t *)arg;
    char buffer[1024];
    int bytes_received;

    // Print initial message with the client's color
    printf("%sClient connected: %d from %s\033[0m\n", colors[client->color], client->sockfd, client->ip_address);

    // Read data from the client
    while ((bytes_received = recv(client->sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        // Broadcast the received message to all clients except the sender
        broadcast_input(buffer, client);
    }

    // Client disconnected
    printf("Client %d from %s disconnected.\n", client->sockfd, client->ip_address);

    // Close the client's socket
    close(client->sockfd);

    // Remove client from the list
    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i] == client) {
            clients[i] = NULL;
            break;
        }
    }
    client_count--;
    pthread_mutex_unlock(&client_mutex);

    free(client);
    return NULL;
}

// Function to accept new clients and create threads
void *accept_clients(void *arg) {
    int server_sock = *(int *)arg;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (1) {
        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("Accept failed");
            continue;
        }

        // Allocate memory for the new client
        client_t *new_client = malloc(sizeof(client_t));
        new_client->sockfd = client_sock;
        new_client->color = client_count % 7; // Cycle through colors

        // Convert client IP to string and store it
        inet_ntop(AF_INET, &client_addr.sin_addr, new_client->ip_address, sizeof(new_client->ip_address));

        // Add the new client to the list
        pthread_mutex_lock(&client_mutex);
        clients[client_count++] = new_client;
        pthread_mutex_unlock(&client_mutex);

        // Create a thread for the new client
        pthread_create(&new_client->thread, NULL, handle_client, (void *)new_client);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP address> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *ip_address = argv[1];
    int port = atoi(argv[2]);

    int server_sock;
    struct sockaddr_in server_addr;

    // Create the server socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    // Convert IP address to network format
    if (inet_pton(AF_INET, ip_address, &server_addr.sin_addr) <= 0) {
        perror("Invalid IP address");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the address
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_sock, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on %s:%d...\n", ip_address, port);

    // Start the thread to accept clients
    pthread_t accept_thread;
    pthread_create(&accept_thread, NULL, accept_clients, (void *)&server_sock);

    // Read from stdin and broadcast to all clients
    char stdin_buffer[1024];
    while (fgets(stdin_buffer, sizeof(stdin_buffer), stdin) != NULL) {
        broadcast_input(stdin_buffer, NULL);
    }

    // Clean up and close server socket
    close(server_sock);
    return 0;
}
