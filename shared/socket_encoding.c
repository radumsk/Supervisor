#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include "./socket_encoding.h"

char* encode_message(char* message, ssize_t message_size){
    char* encoded_message = malloc(message_size + LENGTH_SIZE);
    // Check if message_size is too big
    ssize_t max_size = 1 << (LENGTH_SIZE * 4);
    if(message_size > max_size){
        free(encoded_message);
        return NULL;
    }
    // Add the length of the message to the beginning of the encoded message

    // The length is encoded in hexadecimal
    if(sprintf(encoded_message, "%0*zx", LENGTH_SIZE, message_size) < 0){
        free(encoded_message);
        return NULL;
    }

    // Copy the message
    if(memcpy(encoded_message + LENGTH_SIZE, message, message_size) == NULL){
        free(encoded_message);
        return NULL;
    }

    return encoded_message;
}

bool is_message_complete(char* buffer, ssize_t buffer_size){
    if (buffer_size < LENGTH_SIZE){
        return false;
    }
    // Check if the length of the message is encoded in hexadecimal
    char* temp = malloc(LENGTH_SIZE + 1);
    if(temp == NULL){
        return false;
    }
    if(memcpy(temp, buffer, LENGTH_SIZE) == NULL){
        free(temp);
        return false;
    }
    temp[LENGTH_SIZE] = '\0';
    long length = strtol(temp, NULL, 16);
    free(temp);
    return buffer_size >= length + LENGTH_SIZE;
}

char* decode_message(char* buffer, ssize_t buffer_size){
    // Check if the length of the message is encoded in hexadecimal
    char* temp = malloc(LENGTH_SIZE + 1);
    if(temp == NULL){
        return NULL;
    }
    if(memcpy(temp, buffer, LENGTH_SIZE) == NULL){
        free(temp);
        return NULL;
    }
    temp[LENGTH_SIZE] = '\0';
    long length = strtol(temp, NULL, 16);
    free(temp);

    if(buffer_size < length + LENGTH_SIZE){
        return NULL;
    }

    char* message = malloc(length);
    if(message == NULL){
        return NULL;
    }
    if(memcpy(message, buffer + LENGTH_SIZE, length) == NULL){
        free(message);
        return NULL;
    }
    return message;
}



int send_message(int client_socket, char* message, ssize_t message_size) {
    char *encoded_message = encode_message(message, message_size);
    if (encoded_message == NULL) {
        return errno;
    }

    ssize_t total_sent_bytes = 0;
    do{
        ssize_t sent_bytes = send(client_socket, encoded_message + total_sent_bytes, message_size + LENGTH_SIZE - total_sent_bytes, 0);
        if(sent_bytes == -1){
            perror("send");
            free(encoded_message);
            return errno;
        }
        total_sent_bytes += sent_bytes;
    } while(total_sent_bytes < message_size);

    free(encoded_message);
    return 0;
}

int receive_message(int client_socket, char** message, ssize_t* message_size){
    char* buffer = malloc(BUFFER_SIZE);
    ssize_t read_bytes = 0;
    ssize_t total_read_bytes = 0;
    ssize_t buffer_size = BUFFER_SIZE;
    bool message_complete = false;

    do {
        if(total_read_bytes == buffer_size){
            // Buffer is full, double its size
            buffer_size *= 2;
            char* temp_buffer = realloc(buffer, buffer_size);
            if(temp_buffer == NULL){
                perror("realloc");
                free(buffer);
                return errno;
            }
            buffer = temp_buffer;
        }
        ssize_t bytes_to_read = buffer_size - total_read_bytes; // How many bytes are left until buffer is full
        read_bytes = recv(client_socket, buffer + total_read_bytes, bytes_to_read, 0);
        if(read_bytes == -1){
            perror("recv");
            free(buffer);
            return errno;
        }
        total_read_bytes += read_bytes;
        message_complete = is_message_complete(buffer, total_read_bytes);
        if(!message_complete){
            printf("Message not complete\n");
        }
    } while(!message_complete);

    // Now buffer contains the whole message

    *message = decode_message(buffer, total_read_bytes);
    if(message == NULL){
        free(buffer);
        return errno;
    }
    *message_size = total_read_bytes - LENGTH_SIZE;

    free(buffer);

    return 0;
}