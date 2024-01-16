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

void* listen_on_socket(void* params) {
    bool stop = false;
    int client_socket = ((struct listen_params_t *) params)->client_socket;
    struct listen_return_t* return_value = malloc(sizeof(struct listen_return_t));

    //    free(params);
    while (!stop) {
        char* command;
        ssize_t command_size;
        void* command_params;
        ssize_t params_size;
        if (receive_command(client_socket, &command, &command_size, &command_params, &params_size)) {
            perror("receive_message");
            return_value->error = errno;
            return return_value;
        }

        // DEBUG: Print the message
        printf("Received message: %s\n", command);

        // Check if the message is "stop"
        if (!strncmp(command, "stop", command_size)) {
            if (send_message(client_socket, "ok", 2)) {
                return_value->error = errno;
                return return_value;
            }
            stop = true;
        } else if (!strncmp(command, "service_create", command_size)) {
            struct service_create_args_t* args = deserialize_service_create_args(command_params, params_size);
            if (args == NULL) {
                perror("deserialize_service_create_args");
                return_value->error = errno;
                return return_value;
            }
            service_t service = service_create(args->servicename, args->program_path, args->argv,
                                               args->argc, args->flags);
            if (service == -1) {
                printf("Error creating service\n");
                if (send_error(client_socket, -1)) {
                    return_value->error = errno;
                    return return_value;
                }
            } else {
                printf("Service created\n");
                char* messageBuffer = malloc(2);
                strcpy(messageBuffer, "ok");
                char* paramsBuffer = malloc(sizeof(service_t));
                memcpy(paramsBuffer, &service, sizeof(service_t));
                if (send_command(client_socket, messageBuffer, 2, paramsBuffer, sizeof(service_t))) {
                    return_value->error = errno;
                    return return_value;
                }
            }
            free(command_params);
        } else if (!strncmp(command, "service_close", command_size)) {
            service_t service = *((service_t *) command_params);
            int error = service_close(service);
            if (error) {
                printf("Error closing service\n");
                if (send_error(client_socket, error)) {
                    return_value->error = error;
                    return return_value;
                }
            } else {
                printf("Service closed\n");
                if (send_ok(client_socket)) {
                    return_value->error = errno;
                    return return_value;
                }
            }
        } else if (!strncmp(command, "service_status", command_size)) {
            service_t service = *((service_t *) command_params);
            int status = service_status(service);
            if (status < 0) {
                printf("Error getting status\n");
                if (send_error(client_socket, status)) {
                    return_value->error = status;
                    return return_value;
                }
            } else {
                printf("Service status: %d\n", status);
                char* messageBuffer = malloc(2);
                strcpy(messageBuffer, "ok");
                char* paramsBuffer = malloc(sizeof(int));
                memcpy(paramsBuffer, &status, sizeof(int));
                if (send_command(client_socket, messageBuffer, 2, paramsBuffer, sizeof(int))) {
                    return_value->error = errno;
                    return return_value;
                }
            }
        } else if (!strncmp(command, "service_open", command_size)) {
            int servicename_size = decode_length(command_params);
            char* servicename = malloc(servicename_size + 1);
            memcpy(servicename, command_params + LENGTH_SIZE, servicename_size);
            servicename[servicename_size] = '\0';
            service_t service = service_open(servicename);
            if (service == -1) {
                printf("Error opening service\n");
                if (send_error(client_socket, -1)) {
                    return_value->error = errno;
                    return return_value;
                }
            } else {
                printf("Service opened\n");
                char* messageBuffer = malloc(2);
                strcpy(messageBuffer, "ok");
                char* paramsBuffer = malloc(sizeof(service_t));
                memcpy(paramsBuffer, &service, sizeof(service_t));
                if (send_command(client_socket, messageBuffer, 2, paramsBuffer, sizeof(service_t))) {
                    return_value->error = errno;
                    return return_value;
                }
            }
        } else if(!strncmp(command, "service_suspend", command_size)) {
            service_t service = *((service_t *) command_params);
            int status = service_suspend(service);
            if (status < 0) {
                printf("Error suspending service\n");
                if (send_error(client_socket, status)) {
                    return_value->error = status;
                    return return_value;
                }
            } else {
                printf("Service suspended\n");
                if(send_ok(client_socket)){
                    return_value->error = errno;
                    return return_value;
                }
            }
        } else if(!strncmp(command, "service_resume", command_size)) {
            service_t service = *((service_t *) command_params);
            int status = service_resume(service);
            if (status < 0) {
                printf("Error resuming service\n");
                if (send_error(client_socket, status)) {
                    return_value->error = status;
                    return return_value;
                }
            } else {
                printf("Service resumed\n");
                if(send_ok(client_socket)){
                    return_value->error = errno;
                    return return_value;
                }
            }
        } else if(!strncmp(command, "service_cancel", command_size)) {
            service_t service = *((service_t *) command_params);
            int status = service_cancel(service);
            if (status < 0) {
                printf("Error canceling service\n");
                if (send_error(client_socket, status)) {
                    return_value->error = status;
                    return return_value;
                }
            } else {
                printf("Service canceled\n");
                if(send_ok(client_socket)){
                    return_value->error = errno;
                    return return_value;
                }
            }
        } else if(!strncmp(command, "service_remove", command_size)) {
            service_t service = *((service_t *) command_params);
            int status = service_remove(service);
            if (status < 0) {
                printf("Error removing service\n");
                if (send_error(client_socket, status)) {
                    return_value->error = status;
                    return return_value;
                }
            } else {
                printf("Service removed\n");
                if(send_ok(client_socket)){
                    return_value->error = errno;
                    return return_value;
                }
            }
        } else if(!strncmp(command, "supervisor_list", command_size)) {
            unsigned int count;
            char** service_names;
            int error = supervisor_list(&service_names, &count);
            if (error) {
                printf("Error listing services\n");
                if (send_error(client_socket, error)) {
                    return_value->error = error;
                    return return_value;
                }
            } else {
                printf("Services listed\n");
                char* messageBuffer = malloc(2);
                strcpy(messageBuffer, "ok");
                ssize_t totalBufferLength = sizeof(unsigned int);
                for (int i = 0; i < count; i++) {
                    totalBufferLength += LENGTH_SIZE + strlen(service_names[i]);
                }
                char* paramsBuffer = malloc(totalBufferLength);
                memcpy(paramsBuffer, &count, sizeof(unsigned int));
                ssize_t offset = sizeof(unsigned int);
                for (int i = 0; i < count; i++) {
                    ssize_t service_name_length = strlen(service_names[i]);
                    char* service_name_length_buffer = malloc(LENGTH_SIZE + 1);
                    sprintf(service_name_length_buffer, "%0*zx", LENGTH_SIZE, (int) service_name_length);
                    memcpy(paramsBuffer + offset, service_name_length_buffer, LENGTH_SIZE);
                    offset += LENGTH_SIZE;
                    memcpy(paramsBuffer + offset, service_names[i], service_name_length);
                    offset += service_name_length;
                }
                if (send_command(client_socket, messageBuffer, 2, paramsBuffer, totalBufferLength)) {
                    return_value->error = errno;
                    return return_value;
                }
            }
        } else if(!strncmp(command, "supervisor_freelist", command_size)) {
            // Get count
            int count = decode_length(command_params);
            // Get service names
            char** service_names = malloc(count * sizeof(char*));
            ssize_t offset = LENGTH_SIZE;
            for(int i = 0; i < count; i++){
                int service_name_length = decode_length(command_params + offset);
                offset += LENGTH_SIZE;
                service_names[i] = malloc(service_name_length + 1);
                memcpy(service_names[i], command_params + offset, service_name_length);
                service_names[i][service_name_length] = '\0';
                offset += service_name_length;
            }
            int error = supervisor_freelist(service_names, count);
            if (error) {
                printf("Error freeing list\n");
                if (send_error(client_socket, error)) {
                    return_value->error = error;
                    return return_value;
                }
            } else {
                printf("List freed\n");
                if(send_ok(client_socket)){
                    return_value->error = errno;
                    return return_value;
                }
            }
        }
        else {
            printf("Received unexpected command: %s\n", command);
            if (send_error(client_socket, -1)) {
                return_value->error = errno;
                return return_value;
            }
        }
    }
    // Close the socket
    if (close(client_socket)) {
        perror("close");
        return_value->error = errno;
        return return_value;
    }
    printf("Stopping\n");
    return_value->error = 0;
    stop = true;
    return return_value;
}
