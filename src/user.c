#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "user.h"
#include "encryption.h"
#include "logger.h"

#define USERS_FILE "../dat/user_data.txt"

// Function to check if a user exists in the users file
int user_exists(const char *username)
{
    FILE *file = fopen(USERS_FILE, "r"); // Open users file for reading
    if (file == NULL)
    {
        log_error("Error opening users file for reading");
        return 0;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) // Read each line of the file
    {
        char *stored_username = strtok(line, ":");  // Extract username from the line
        if (strcmp(username, stored_username) == 0)  // Compare usernames
        {
            fclose(file);
            return 1; // User found
        }
    }

    fclose(file);
    return 0; // User not found
}

// Function to register a new user
int register_user(const char *username, const char *password)
{
    if (user_exists(username)) // Check if user already exists
    {
        log_warning("User already exists");
        return 0; // Registration failed
    }
    FILE *file = fopen(USERS_FILE, "a");  // Open users file for appending
    if (file == NULL)
    {
        log_error("Error opening users file");
        return 0;
    }

    char encrypted_password[100];
    custom_encrypt(password, encrypted_password); // Encrypt password

    fprintf(file, "%s:%s\n", username, encrypted_password); // Write username and encrypted password to file
    fclose(file);

    log_info("User registered successfully");
    return 1; // Registration successful
}

// Function to authenticate a user
int authenticate_user(const char *username, const char *password)
{
    FILE *file = fopen(USERS_FILE, "r");  // Open users file for reading
    if (file == NULL)
    {
        log_error("Error opening users file");
        return 0;
    }

    char line[256];
    log_debug("Attempting to authenticate user");
    log_debug(username);
    while (fgets(line, sizeof(line), file))  // Read each line of the file
    {
        log_debug(line);
        char *stored_username = strtok(line, ":");    // Extract username from the line
        char *stored_password = strtok(NULL, "\n");      // Extract password from the line

        if (strcmp(username, stored_username) == 0)      // Compare usernames
        {
            char decrypted_password[100];
            custom_decrypt(stored_password, decrypted_password); // Decrypt stored password
            if (strcmp(password, decrypted_password) == 0)     // Compare passwords
            {
                fclose(file);
                return 1;         // Authentication successful

            }
        }
    }

    fclose(file);   // Authentication failed
    return 0;
}


