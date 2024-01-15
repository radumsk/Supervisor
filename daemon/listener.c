#include <stdbool.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
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
        } else if(!strcmp(command, "service_create")) {
            struct service_create_args_t* args = deserialize_service_create_args(command_params, params_size);
            if(args == NULL){
                perror("deserialize_service_create_args");
                return_value->error = errno;
                return return_value;
            }
            service_t service = service_create(args->supervisor, args->servicename, args->program_path, args->argv, args->argc, args->flags);
            if(service == -1){
                printf("Error creating service\n");
                if(send_message(client_socket, "error", 5)){
                    return_value->error = errno;
                    return return_value;
                }
            } else {
                printf("Service created\n");
                if(send_command(client_socket, "ok", 2, &service, sizeof(service_t))){
                    return_value->error = errno;
                    return return_value;
                }
            }
            free(command_params);
        } else if(!strcmp(command, "service_close")) {
            service_t service = *((service_t*) command_params);
            if(service_close(service)){
                printf("Error closing service\n");
                if(send_message(client_socket, "error", 5)){
                    return_value->error = errno;
                    return return_value;
                }
            } else {
                printf("Service closed\n");
                if(send_message(client_socket, "ok", 2)){
                    return_value->error = errno;
                    return return_value;
                }
            }
        }
        /*
        else if (!strcmp(command, "suspend")){

            if(send_message(client_socket, "ok", 2)){
                return_value->error = errno;
                return return_value;
            }

            kill(getpid(), SIGSTOP);
        }

        else if (!strcmp(command, "resume")){

            if(send_message(client_socket, "ok", 2)){
                return_value->error = errno;
                return return_value;
            }

            kill(getpid(), SIGCONT);
        }

        else if(!strcmp(command, "cancel")) {

            if(send_message(client_socket, "ok", 2)){
                return_value->error = errno;
                return return_value;
            }

            int result = kill(getpid(), SIGKILL);
            if (result) {
                perror("kill");
                return_value->error = errno;
                return return_value;
            }
        }
        else if(!strcmp(command, "remove")){
            // inchide handlerul obtinut cu service_open() si elibereaza resursele asociate

            if(send_message(client_socket, "ok", 2)){
                return_value->error = errno;
                return return_value;
            }
            free(command_params);
            free(command);
            stop = true;

        }
         */
        else {
            printf("Received unexpected command: %s\n", command);
            if(send_message(client_socket, "error", 5)){
                return_value->error = errno;
                return return_value;
            }
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