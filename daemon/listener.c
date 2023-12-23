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
        char* command;
        ssize_t command_size;
        void* command_params;
        ssize_t params_size;
        if(receive_command(client_socket, &command, &command_size, &command_params, &params_size)){
            perror("receive_message");
            return_value->error = errno;
            return return_value;
        }

        // DEBUG: Print the message
        printf("Received message: %s\n", command);

        // Check if the message is "stop"
        if(!strcmp(command, "stop")){
            if(send_message(client_socket, "ok", 2)){
                return_value->error = errno;
                return return_value;
            }
            stop = true;
        } else if(!strcmp(command, "bebino")){
            print_bebino(client_socket, command_params);
            free(command_params);
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