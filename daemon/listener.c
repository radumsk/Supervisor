#include <stdbool.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "daemon_flags.h"
#include "../shared/socket_encoding.h"
#include "listener.h"
#include "methods.h"

extern pthread_mutex_t mutex;

void* listen_on_socket(void* params){
    bool stop = false;
    int client_socket = ((struct listen_params_t*) params)->client_socket;
    struct listen_return_t* return_value = malloc(sizeof(struct listen_return_t));

//    free(params);
    while (!stop){
        char* message;
        ssize_t message_size;
        if(receive_message(client_socket, &message, &message_size)){
            perror("receive_message");
            return_value->error = errno;
            return return_value;
        }

        // DEBUG: Print the message
        printf("%s\n", message);

        // Check if the message is "stop"
        if(!strcmp(message, "stop")){
            if(send_message(client_socket, "ok", 2)){
                return_value->error = errno;
                return return_value;
            }
            stop = true;
        } else if(!strcmp(message, "bebino")){
            print_bebino(client_socket);
        }
    }
    // Close the socket
    if(close(client_socket)){
        perror("close");
        return_value->error = errno;
        return return_value;
    }
    printf("Stopping\n");
    return_value->error = 0;
    return return_value;
}