#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdbool.h>
#include "server.h"
#include "user.h"
#include "encryption.h"
#include "logger.h"

#define MAX_MESSAGE_LENGTH 1024

// Function to broadcast a message to all clients except the sender
void broadcast_message(chat_server_t *server, char *message, int sender_socket)
{
    pthread_mutex_lock(&server->clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (server->clients[i] && server->clients[i]->socket != sender_socket)
        {
            char encrypted_msg[MAX_MESSAGE_LENGTH];
            custom_encrypt(message, encrypted_msg);
            send(server->clients[i]->socket, encrypted_msg, strlen(encrypted_msg), 0);
        }
    }
    pthread_mutex_unlock(&server->clients_mutex);
}

// Function to remove a client from the server
void remove_client(chat_server_t *server, client_t *client)
{
    pthread_mutex_lock(&server->clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (server->clients[i] == client)
        {
            server->clients[i] = NULL;
            server->client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&server->clients_mutex);

    close(client->socket);
    free(client);
}

// Function to handle client connections
void *handle_client(void *arg)
{
    client_t *client = (client_t *)arg;
    chat_server_t *server = (chat_server_t *)client->server;
    char buffer[MAX_MESSAGE_LENGTH];
    int read_size;

    // Wait for the username from the client
    read_size = recv(client->socket, buffer, sizeof(buffer) - 1, 0);
    if (read_size > 0)
    {
        buffer[read_size] = '\0';
        if (strncmp(buffer, "USERNAME:", 9) == 0)
        {
            // Extract and store the username
            strncpy(client->username, buffer + 9, sizeof(client->username) - 1);
            client->username[sizeof(client->username) - 1] = '\0';
            log_info("User %s connected from IP %s", client->username, client->ip_address);
        }
    }

    // Handle incoming messages from the client
    while ((read_size = recv(client->socket, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
        buffer[read_size] = '\0';
        char decrypted_msg[MAX_MESSAGE_LENGTH];
        custom_decrypt(buffer, decrypted_msg);

        // Prepare broadcast message format including username and IP
        char broadcast_buffer[MAX_MESSAGE_LENGTH + 100];
        snprintf(broadcast_buffer, sizeof(broadcast_buffer), "%s (%s): %s", client->username, client->ip_address, decrypted_msg);
        
        // Broadcast the message to all clients except the sender
        broadcast_message(server, broadcast_buffer, client->socket);
    }

    // Client disconnected handling
    if (read_size == 0)
    {
        log_info("Client %s disconnected", client->username);
    }
    else if (read_size == -1)
    {
        log_error("Receive failed from client %s", client->username);
    }

    // Remove the client from the server
    remove_client(server, client);

    // If no more clients are connected, stop the server
    if (server->client_count == 0)
    {
        log_info("No more clients on server %d. Stopping server.", server->id);
        server->running = false;
	pthread_exit(NULL);
    }

    return NULL;
}

// Function to run the server and handle incoming client connections
void *run_server(void *arg)
{
    chat_server_t *server = (chat_server_t *)arg;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);

    // Create server socket
    server->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server->server_socket == -1)
    {
        log_fatal("Error creating server socket for server %d", server->id);
        return NULL;
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(server->port);

    // Bind server socket
    if (bind(server->server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        log_fatal("Error binding server socket for server %d", server->id);
        return NULL;
    }

    // Start listening for incoming connections
    if (listen(server->server_socket, 10) < 0)
    {
        log_fatal("Error listening on server socket for server %d", server->id);
        return NULL;
    }

    log_info("Server %d started on port %d. Waiting for connections...", server->id, server->port);

<<<<<<< HEAD
    inline void handle_sigint(int sig){
    	stop_chat_server(server);
    }
    signal(SIGINT,handle_sigint);
    
=======
    // Accept and handle incoming client connections
>>>>>>> 182c2d64a32d6b2409afb8d20224a3cca624eb09
    while (server->running)
    {
        int client_socket = accept(server->server_socket, (struct sockaddr *)&client_addr, &addr_size);
        if (client_socket < 0)
        {
            if (server->running)
            {
                log_error("Error accepting client connection on server %d", server->id);
            }
            continue;
        }

        // Create a client structure and initialize it
        client_t *client = malloc(sizeof(client_t));
        if (client == NULL)
        {
            log_error("Failed to allocate memory for client on server %d", server->id);
            close(client_socket);
            continue;
        }

        client->socket = client_socket;
        client->address = client_addr;
        client->server = server;
        inet_ntop(AF_INET, &(client->address.sin_addr), client->ip_address, INET_ADDRSTRLEN);

        // Add client to the server's client list
        pthread_mutex_lock(&server->clients_mutex);
        int i;
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            if (server->clients[i] == NULL)
            {
                server->clients[i] = client;
                server->client_count++;
                break;
            }
        }
        pthread_mutex_unlock(&server->clients_mutex);

        // Handle case when server is full
        if (i == MAX_CLIENTS)
        {
            log_error("Server %d is full, connection rejected", server->id);
            close(client_socket);
            free(client);
            continue;
        }

        // Create a thread to handle the client
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void *)client) != 0)
        {
            log_error("Error creating thread for client on server %d", server->id);
            remove_client(server, client);
        }
        else
        {
            pthread_detach(thread_id);
        }
    }

    // Server shutdown handling
    log_info("Server %d on port %d is shutting down", server->id, server->port);
    close(server->server_socket);

    return NULL;
}

// Function to create and initialize a chat server
chat_server_t *create_chat_server(int id, int port)
{
    log_debug("Creating chat server with id %d and port %d", id, port);
    if (id <= 0 || port <= 0 || port > 65535) {
        log_error("Invalid id or port number");
        return NULL;
    }
    chat_server_t *server = malloc(sizeof(chat_server_t));
    if (server == NULL)
    {
        log_error("Failed to allocate memory for chat server");
        return NULL;
    }

    // Initialize server attributes
    server->id = id;
    server->port = port;
    server->client_count = 0;
    server->running = true;

    // Initialize clients mutex
    if (pthread_mutex_init(&server->clients_mutex, NULL) != 0) {
        log_error("Failed to initialize clients mutex");
        free(server);
        return NULL;
    }

    // Initialize clients array
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        server->clients[i] = NULL;
    }

    // Create a thread to run the server
    if (pthread_create(&server->thread, NULL, run_server, (void *)server) != 0)
    {
        log_error("Failed to create thread for chat server %d", id);
        pthread_mutex_destroy(&server->clients_mutex);
        free(server);
        return NULL;
    }

    log_debug("Chat server thread created successfully.");
    return server;
}
<<<<<<< HEAD

void stop_chat_server(chat_server_t *server)
{
    server->running = false;
    shutdown(server->server_socket, SHUT_RDWR);
    close(server->server_socket);

    pthread_join(server->thread, NULL);

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (server->clients[i])
        {
            close(server->clients[i]->socket);
            free(server->clients[i]);
        }
    }

    pthread_mutex_destroy(&server->clients_mutex);
    log_info("Server %d on port %d has been stopped", server->id, server->port);
exit(EXIT_SUCCESS);
}
=======
>>>>>>> 182c2d64a32d6b2409afb8d20224a3cca624eb09
