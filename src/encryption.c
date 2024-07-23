#include <string.h>
#include "encryption.h"

// Simple XOR encryption for demonstration purposes

#define KEY 0x73

void custom_encrypt(const char *input, char *output)
{
    int len = strlen(input);
    for (int i = 0; i < len; i++)
    {
        output[i] = input[i] ^ KEY;
    }
    output[len] = '\0';
}

void custom_decrypt(const char *input, char *output)
{
    int len = strlen(input);
    for (int i = 0; i < len; i++)
    {
        output[i] = input[i] ^ KEY;
    }
    output[len] = '\0';
}

