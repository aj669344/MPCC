#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server.h"
#include "client.h"
#include "logger.h"
#include <unistd.h>

int main(int argc, char *argv[])
{
    // Check if the correct number of command-line arguments are provided
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <mode> [port]\n", argv[0]);
        fprintf(stderr, "mode: 'server' or 'client'\n");
        exit(1); // Exit with error code 1 if arguments are insufficient
    }

    // Initialize the logger
    if (init_logger() != 0)
    {
        fprintf(stderr, "Failed to initialize logger\n");
        exit(1); // Exit with error code 1 if logger initialization fails
    }

    // Check if the mode is "server"
    if (strcmp(argv[1], "server") == 0)
    {
        // If mode is server, check if the port number is provided
        if (argc != 3)
        {
            fprintf(stderr, "Usage: %s server <port>\n", argv[0]);
            exit(1); // Exit with error code 1 if port number is missing
        }

        // Convert port argument from string to integer
        int port = atoi(argv[2]);
        // Validate the port number
        if (port <= 0 || port > 65535)
        {
            log_fatal("Invalid port number: %d", port);
            exit(1); // Exit with error code 1 if port number is invalid
        }

        // Create a chat server using the provided port
        chat_server_t *server = create_chat_server(1, port);
        if (server == NULL)
        {
            log_fatal("Failed to create chat server");
            exit(1); // Exit with error code 1 if server creation fails
        }

        // Log successful creation of chat server
        log_info("Chat server created on port %d", port);

        // Server loop: Typically servers run indefinitely waiting for connections
        while (1)
        {
            sleep(1); // Sleep for 1 second before checking again
        }
    }
    else if (strcmp(argv[1], "client") == 0)
    {
        // If mode is client, execute the client logic
        run_client();
    }
    else
    {
        // If mode is neither "server" nor "client", log an error and exit
        log_fatal("Invalid mode. Use 'server' or 'client'.");
        exit(1); // Exit with error code 1 for invalid mode
    }

    return 0; // Exit normally (0) after successful execution
}
