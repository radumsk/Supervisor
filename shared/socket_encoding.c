#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include "./socket_encoding.h"

char *encode_message(char *message, ssize_t message_size) {
    char *encoded_message = malloc(message_size + LENGTH_SIZE);
    // Check if message_size is too big
    ssize_t max_size = 1 << (LENGTH_SIZE * 4);
    if (message_size > max_size) {
        free(encoded_message);
        return NULL;
    }
    // Add the length of the message to the beginning of the encoded message

    // The length is encoded in hexadecimal
    if (sprintf(encoded_message, "%0*zx", LENGTH_SIZE, message_size) < 0) {
        free(encoded_message);
        return NULL;
    }

    // Copy the message
    if (memcpy(encoded_message + LENGTH_SIZE, message, message_size) == NULL) {
        free(encoded_message);
        return NULL;
    }

    return encoded_message;
}

bool is_message_complete(char *buffer, ssize_t buffer_size) {
    if (buffer_size < LENGTH_SIZE) {
        return false;
    }
    // Check if the length of the message is encoded in hexadecimal
    char *temp = malloc(LENGTH_SIZE + 1);
    if (temp == NULL) {
        return false;
    }
    if (memcpy(temp, buffer, LENGTH_SIZE) == NULL) {
        free(temp);
        return false;
    }
    temp[LENGTH_SIZE] = '\0';
    long length = strtol(temp, NULL, 16);
    free(temp);
    return buffer_size >= length + LENGTH_SIZE;
}

char *decode_message(char *buffer, ssize_t buffer_size) {
    // Check if the length of the message is encoded in hexadecimal
    char *temp = malloc(LENGTH_SIZE + 1);
    if (temp == NULL) {
        return NULL;
    }
    if (memcpy(temp, buffer, LENGTH_SIZE) == NULL) {
        free(temp);
        return NULL;
    }
    temp[LENGTH_SIZE] = '\0';
    long length = strtol(temp, NULL, 16);
    free(temp);

    if (buffer_size < length + LENGTH_SIZE) {
        return NULL;
    }

    char *message = malloc(length);
    if (message == NULL) {
        return NULL;
    }
    if (memcpy(message, buffer + LENGTH_SIZE, length) == NULL) {
        free(message);
        return NULL;
    }
    return message;
}


int send_message(int client_socket, char *message, ssize_t message_size) {
    char *encoded_message = encode_message(message, message_size);
    if (encoded_message == NULL) {
        return errno;
    }

    ssize_t total_sent_bytes = 0;
    do {
        ssize_t sent_bytes = send(client_socket, encoded_message + total_sent_bytes,
                                  message_size + LENGTH_SIZE - total_sent_bytes, 0);
        if (sent_bytes == -1) {
            perror("send");
            free(encoded_message);
            return errno;
        }
        total_sent_bytes += sent_bytes;
    } while (total_sent_bytes < message_size);

    free(encoded_message);
    return 0;
}

int receive_message(int client_socket, char **message, ssize_t *message_size) {
    char *buffer = malloc(BUFFER_SIZE);
    ssize_t read_bytes = 0;
    ssize_t total_read_bytes = 0;
    ssize_t buffer_size = BUFFER_SIZE;
    bool message_complete = false;

    do {
        if (total_read_bytes == buffer_size) {
            // Buffer is full, double its size
            buffer_size *= 2;
            char *temp_buffer = realloc(buffer, buffer_size);
            if (temp_buffer == NULL) {
                perror("realloc");
                free(buffer);
                return errno;
            }
            buffer = temp_buffer;
        }
        ssize_t bytes_to_read = buffer_size - total_read_bytes; // How many bytes are left until buffer is full
        read_bytes = recv(client_socket, buffer + total_read_bytes, bytes_to_read, 0);
        if (read_bytes == -1) {
            perror("recv");
            free(buffer);
            return errno;
        }
        total_read_bytes += read_bytes;
        message_complete = is_message_complete(buffer, total_read_bytes);
        if (!message_complete) {
            printf("Message not complete\n");
        }
    } while (!message_complete);

    // Now buffer contains the whole message

    *message = decode_message(buffer, total_read_bytes);
    if (message == NULL) {
        free(buffer);
        return errno;
    }
    *message_size = total_read_bytes - LENGTH_SIZE;

    free(buffer);

    return 0;
}

int send_command(int socket, char *command, ssize_t command_size, void *params, ssize_t params_size) {
    char *buffer = malloc(command_size + params_size + LENGTH_SIZE);
    if (buffer == NULL) {
        perror("malloc");
        return errno;
    }
    // Copy command to buffer
    if (memcpy(buffer, command, command_size) == NULL) {
        perror("memcpy");
        free(buffer);
        return errno;
    }
    // Add command delimiter
    buffer[command_size] = ':';
    // Copy params to buffer
    if (params_size > 0 && params != NULL) {
        if (memcpy(buffer + command_size + 1, params, params_size) == NULL) {
            perror("memcpy");
            free(buffer);
            return errno;
        }
    }
    // Send
    if (send_message(socket, buffer, command_size + params_size + 1)) {
        perror("send_message");
        free(buffer);
        return errno;
    }
    free(buffer);
    return 0;
}

int receive_command(int socket, char **command, ssize_t *command_size, void **params, ssize_t *params_size) {
    char *buffer;
    ssize_t buffer_size;
    if (receive_message(socket, &buffer, &buffer_size)) {
        perror("receive_message");
        return errno;
    }
    // Find the command delimiter
    ssize_t delimiter_index = 0;
    while (buffer[delimiter_index] != ':') {
        delimiter_index++;
        if (delimiter_index == buffer_size) {
            printf("Command delimiter not found\n");
            return -1;
        }
    }
    // Copy command to command
    *command = malloc(delimiter_index);
    if (*command == NULL) {
        perror("malloc");
        return errno;
    }
    if (memcpy(*command, buffer, delimiter_index) == NULL) {
        perror("memcpy");
        free(*command);
        return errno;
    }
    *command_size = delimiter_index;
    // Copy params to params
    *params_size = buffer_size - delimiter_index - 1;
    *params = malloc(*params_size);
    if (*params == NULL) {
        perror("malloc");
        free(*command);
        return errno;
    }
    if (memcpy(*params, buffer + delimiter_index + 1, *params_size) == NULL) {
        perror("memcpy");
        free(*command);
        free(*params);
        return errno;
    }
    free(buffer);
    return 0;
}

int decode_length (const char *buffer) {
    char *temp = malloc(LENGTH_SIZE + 1);
    if (temp == NULL) {
        return -1;
    }
    if (memcpy(temp, buffer, LENGTH_SIZE) == NULL) {
        free(temp);
        return -1;
    }
    temp[LENGTH_SIZE] = '\0';
    long length = strtol(temp, NULL, 16);
    free(temp);
    return length;
}