#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include "client.h"
#include "user.h"
#include "encryption.h"
#include "logger.h"

#define MAX_IP_LENGTH 46
#define MAX_USERNAME_LENGTH 50
#define MAX_PASSWORD_LENGTH 50
#define MAX_MESSAGE_LENGTH 1024

static int client_socket;
static char username[MAX_USERNAME_LENGTH];
static volatile int running = 1;

// Function to handle receiving messages from the server
static void *receive_messages(void *arg)
{
    char buffer[MAX_MESSAGE_LENGTH];
    int read_size;

    while (running)
    {
        memset(buffer, 0, sizeof(buffer));
        read_size = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

        if (read_size > 0)
        {
            buffer[read_size] = '\0';
            // Check for server shutdown signal
            if (strcmp(buffer, "SERVER_SHUTDOWN") == 0)
            {
                printf("Server has been shut down. Disconnecting...\n");
                running = 0;
                break;
            }
            // Decrypt and print received message
            char decrypted_msg[MAX_MESSAGE_LENGTH];
            custom_decrypt(buffer, decrypted_msg);
            printf("%s\n", decrypted_msg);
        }
        else if (read_size == 0)
        {
            printf("Server disconnected\n");
            running = 0;
            break;
        }
        else
        {
            if (running)
            {
                log_error("Receive failed");
                running = 0;
                break;
            }
        }
    }

    log_info("Receive thread ending");
    return NULL;
}

// Function to run the client application
void run_client()
{
    char SERVER_IP[MAX_IP_LENGTH];
    int PORT;
    struct sockaddr_in server_addr;

    log_info("Starting client");

    // Read server IP from user
    printf("Enter server IP: ");
    if (fgets(SERVER_IP, sizeof(SERVER_IP), stdin) == NULL)
    {
        log_error("Error reading server IP");
        return;
    }
    SERVER_IP[strcspn(SERVER_IP, "\n")] = 0;

    // Read server port from user
    printf("Enter server port: ");
    char port_str[6];
    if (fgets(port_str, sizeof(port_str), stdin) == NULL)
    {
        log_error("Error reading port");
        return;
    }
    PORT = atoi(port_str);
    if (PORT <= 0 || PORT > 65535)
    {
        log_error("Invalid port number");
        return;
    }

    // Create client socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        log_fatal("Error creating client socket");
        return;
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        log_fatal("Error connecting to server");
        close(client_socket);
        return;
    }

    log_info("Connected to server");

    // Read username from user
    printf("Enter username: ");
    if (fgets(username, sizeof(username), stdin) == NULL)
    {
        log_error("Error reading username");
        close(client_socket);
        return;
    }
    username[strcspn(username, "\n")] = 0;

    // Read password from user
    char password[MAX_PASSWORD_LENGTH];
    printf("Enter password: ");
    if (fgets(password, sizeof(password), stdin) == NULL)
    {
        log_error("Error reading password");
        close(client_socket);
        return;
    }
    password[strcspn(password, "\n")] = 0;

    // Check if user wants to register
    printf("Do you want to register as a new user? (y/n): ");
    char register_choice[2];
    if (fgets(register_choice, sizeof(register_choice), stdin) == NULL)
    {
        log_error("Error reading registration choice");
        close(client_socket);
        return;
    }
    if (register_choice[0] == 'y' || register_choice[0] == 'Y')
    {
        // Attempt to register the user
        if (register_user(username, password))
        {
            log_info("User registered successfully");
        }
        else
        {
            log_error("User registration failed. User may already exist.");
            close(client_socket);
            return;
        }
    }

    // Authenticate the user
    if (!authenticate_user(username, password))
    {
        log_warning("Authentication failed");
        close(client_socket);
        return;
    }

    log_info("Authentication successful");

    // Send username to server
    char username_msg[MAX_MESSAGE_LENGTH];
    snprintf(username_msg, sizeof(username_msg), "USERNAME:%s", username);
    send(client_socket, username_msg, strlen(username_msg), 0);

    // Create thread to receive messages from server
    pthread_t receive_thread;
    if (pthread_create(&receive_thread, NULL, receive_messages, NULL) != 0)
    {
        log_error("Error creating receive thread");
        close(client_socket);
        return;
    }

    char message[MAX_MESSAGE_LENGTH];
    while (running)
    {
        // Read user input message
        if (fgets(message, sizeof(message), stdin) == NULL)
        {
            if (running)
            {
                log_error("Error reading message");
            }
            break;
        }
        message[strcspn(message, "\n")] = 0;

        // Check if user wants to exit
        if (strcmp(message, "exit") == 0)
        {
            running = 0;
            break;
        }

        // Encrypt and send message to server
        char encrypted_msg[MAX_MESSAGE_LENGTH];
        custom_encrypt(message, encrypted_msg);
        if (send(client_socket, encrypted_msg, strlen(encrypted_msg), 0) < 0)
        {
            log_error("Send failed");
            running = 0;
            break;
        }
    }

    log_info("Closing client connection");
    close(client_socket);
    pthread_cancel(receive_thread);
    pthread_join(receive_thread, NULL);
    printf("Disconnected from server.\n");
}
