//
// Created by radu on 12/22/23.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#include "service.h"
#include "../shared/socket_encoding.h"

char* serialize_service_create_args(struct service_create_args_t args, int* size) {
    // Allocate memory
    size_t totalBufferSize = 0;
    totalBufferSize += sizeof(args.supervisor);
    totalBufferSize += LENGTH_SIZE;
    const size_t servicenameLength = strlen(args.servicename);
    totalBufferSize += servicenameLength;
    totalBufferSize += LENGTH_SIZE;
    const size_t programpathLength = strlen(args.program_path);
    totalBufferSize += programpathLength;
    totalBufferSize += sizeof(args.flags);
    totalBufferSize += sizeof(args.argc);
    for (int i = 0; i < args.argc; i++) {
        totalBufferSize += LENGTH_SIZE;
        totalBufferSize += strlen(args.argv[i]);
    }
    char* buffer = malloc(totalBufferSize);
    // Copy into memory
    char* currentBufferPosition = buffer;
    memcpy(currentBufferPosition, &args.supervisor, sizeof(args.supervisor));
    currentBufferPosition += sizeof(args.supervisor);
    // Copy size of servicename
    if (sprintf(currentBufferPosition, "%0*zx", LENGTH_SIZE, servicenameLength) < 0) {
        free(buffer);
        return NULL;
    }
    currentBufferPosition += LENGTH_SIZE;
    // Copy servicename
    memcpy(currentBufferPosition, args.servicename, servicenameLength);
    currentBufferPosition += servicenameLength;
    // Copy size of programpath
    if (sprintf(currentBufferPosition, "%0*zx", LENGTH_SIZE, programpathLength) < 0) {
        free(buffer);
        return NULL;
    }
    currentBufferPosition += LENGTH_SIZE;
    // Copy programpath
    memcpy(currentBufferPosition, args.program_path, programpathLength);
    currentBufferPosition += programpathLength;
    // Copy flags
    memcpy(currentBufferPosition, &args.flags, sizeof(args.flags));
    currentBufferPosition += sizeof(args.flags);
    // Copy argc
    memcpy(currentBufferPosition, &args.argc, sizeof(args.argc));
    currentBufferPosition += sizeof(args.argc);
    // Copy argv
    for (int i = 0; i < args.argc; i++) {
        const size_t argLength = strlen(args.argv[i]);
        if (sprintf(currentBufferPosition, "%0*zx", LENGTH_SIZE, argLength) < 0) {
            free(buffer);
            return NULL;
        }
        currentBufferPosition += LENGTH_SIZE;
        memcpy(currentBufferPosition, args.argv[i], argLength);
        currentBufferPosition += argLength;
    }
    *size = totalBufferSize;
    return buffer;
}


service_t service_create(
    supervisor_t supervisor,
    const char* servicename,
    const char* program_path,
    const char** argv,
    int argc,
    int flags
) {
    char* command = "service_create";
    char* commandBuffer = malloc(strlen(command) + 1);
    if (commandBuffer == NULL) {
        return -1;
    }
    strcpy(commandBuffer, command);
    char* params;
    int params_size;
    struct service_create_args_t args = {
        .supervisor = supervisor,
        .servicename = servicename,
        .program_path = program_path,
        .argv = argv,
        .argc = argc,
        .flags = flags
    };
    params = serialize_service_create_args(args, &params_size);
    if (params == NULL) {
        perror("serialize_service_create_args");
        return -1;
    }

    if(send_command(supervisor, commandBuffer, strlen(command), params, params_size)){
        perror("send_command");
        return -1;
    }

    //Receive response from daemon

    service_t* response;
    ssize_t response_size;
    char* command_response;
    ssize_t command_response_size;

    if(receive_command(supervisor, &command_response, &command_response_size, &response, &response_size)){
        perror("receive_command");
        return -1;
    }

    if(strcmp(command_response, "ok") != 0){
        printf("Received unexpected response: %s\n", command_response);
        return -1;
    }

    return *response;
}
int service_close(supervisor_t supervisor, service_t service) {

    char* command = malloc(strlen("service_close") + 1);
    if (command == NULL) {
        return -1;
    }
    strcpy(command, "service_close");
    char* params = malloc(sizeof(service_t));
    if (params == NULL) {
        free(command);
        return -1;
    }
    memcpy(params, &service, sizeof(service_t));
    if(send_command(supervisor, command, strlen(command), params, sizeof(service))){
        perror("send_command");
        return -1;
    }
    //Receive response from daemon

    char* command_response;
    ssize_t command_response_size;
    char* response;
    ssize_t response_size;
    if(receive_command(supervisor, &command_response, &command_response_size, &response, &response_size)){
        perror("receive_command");
        return -1;
    }

    if(strcmp(command_response, "ok") != 0){
        printf("Received error response: %s\n", response);
        return -1;
    }
}

int service_status(supervisor_t supervisor, service_t service) {
    char* command = malloc(strlen("service_status") + 1);
    if (command == NULL) {
        return -1;
    }
    strcpy(command, "service_status");
    char* params = malloc(sizeof(service_t));
    if (params == NULL) {
        free(command);
        return -1;
    }
    memcpy(params, &service, sizeof(service_t));
    if(send_command(supervisor, command, strlen(command), params, sizeof(service))){
        perror("send_command");
        return -1;
    }
    //Receive response from daemon

    char* command_response;
    ssize_t command_response_size;
    char* response;
    ssize_t response_size;
    if(receive_command(supervisor, &command_response, &command_response_size, &response, &response_size)){
        perror("receive_command");
        return -1;
    }

    if(strcmp(command_response, "ok") != 0){
        printf("Received error response: %s\n", response);
        return -1;
    }
    return *response;
}

service_t service_open(supervisor_t supervisor, const char* servicename) {
    char* command = malloc(strlen("service_open") + 1);
    if (command == NULL) {
        return -1;
    }
    strcpy(command, "service_open");
    char* params = malloc(strlen(servicename) + LENGTH_SIZE);
    // Copy size of servicename
    if (sprintf(params, "%0*zx", LENGTH_SIZE, strlen(servicename)) < 0) {
        free(command);
        return -1;
    }
    // Copy servicename
    memcpy(params + LENGTH_SIZE, servicename, strlen(servicename));
    if(send_command(supervisor, command, strlen(command), params, strlen(servicename) + LENGTH_SIZE)){
        perror("send_command");
        return -1;
    }
    //Receive response from daemon
    service_t* response;
    ssize_t response_size;
    char* command_response;
    ssize_t command_response_size;
    if(receive_command(supervisor, &command_response, &command_response_size, &response, &response_size)){
        perror("receive_command");
        return -1;
    }

    return *response;
}

int service_suspend(supervisor_t supervisor, service_t service) {
    char* command = malloc(strlen("service_suspend") + 1);
    if (command == NULL) {
        return -1;
    }
    strcpy(command, "service_suspend");
    char* params = malloc(sizeof(service_t));
    if (params == NULL) {
        free(command);
        return -1;
    }
    memcpy(params, &service, sizeof(service_t));
    if(send_command(supervisor, command, strlen(command), params, sizeof(service))){
        perror("send_command");
        return -1;
    }
    //Receive response from daemon

    char* command_response;
    ssize_t command_response_size;
    char* response;
    ssize_t response_size;
    if(receive_command(supervisor, &command_response, &command_response_size, &response, &response_size)){
        perror("receive_command");
        return -1;
    }

    if(strcmp(command_response, "ok") != 0){
        printf("Received error response: %s\n", response);
        return -1;
    }

    return 0;
}

int service_resume(supervisor_t supervisor, service_t service) {
    char* command = malloc(strlen("service_resume") + 1);
    if (command == NULL) {
        return -1;
    }
    strcpy(command, "service_resume");
    char* params = malloc(sizeof(service_t));
    if (params == NULL) {
        free(command);
        return -1;
    }
    memcpy(params, &service, sizeof(service_t));
    if(send_command(supervisor, command, strlen(command), params, sizeof(service))){
        perror("send_command");
        return -1;
    }
    //Receive response from daemon

    char* command_response;
    ssize_t command_response_size;
    char* response;
    ssize_t response_size;
    if(receive_command(supervisor, &command_response, &command_response_size, &response, &response_size)){
        perror("receive_command");
        return -1;
    }

    if(strcmp(command_response, "ok") != 0){
        printf("Received error response: %s\n", response);
        return -1;
    }

    return 0;
}

int service_cancel(supervisor_t supervisor, service_t service) {
    char* command = malloc(strlen("service_cancel") + 1);
    if (command == NULL) {
        return -1;
    }
    strcpy(command, "service_cancel");
    char* params = malloc(sizeof(service_t));
    if (params == NULL) {
        free(command);
        return -1;
    }
    memcpy(params, &service, sizeof(service_t));
    if(send_command(supervisor, command, strlen(command), params, sizeof(service))){
        perror("send_command");
        return -1;
    }
    //Receive response from daemon

    char* command_response;
    ssize_t command_response_size;
    char* response;
    ssize_t response_size;
    if(receive_command(supervisor, &command_response, &command_response_size, &response, &response_size)){
        perror("receive_command");
        return -1;
    }

    if(strcmp(command_response, "ok") != 0){
        printf("Received error response: %s\n", response);
        return -1;
    }

    return 0;
}

int service_remove(supervisor_t supervisor, service_t service) {
    char* command = malloc(strlen("service_remove") + 1);
    if (command == NULL) {
        return -1;
    }
    strcpy(command, "service_remove");
    char* params = malloc(sizeof(service_t));
    if (params == NULL) {
        free(command);
        return -1;
    }
    memcpy(params, &service, sizeof(service_t));
    if(send_command(supervisor, command, strlen(command), params, sizeof(service))){
        perror("send_command");
        return -1;
    }
    //Receive response from daemon

    char* command_response;
    ssize_t command_response_size;
    char* response;
    ssize_t response_size;
    if(receive_command(supervisor, &command_response, &command_response_size, &response, &response_size)){
        perror("receive_command");
        return -1;
    }

    if(strcmp(command_response, "ok") != 0){
        printf("Received error response: %s\n", response);
        return -1;
    }

    return 0;
}

int supervisor_list(supervisor_t supervisor, char*** service_names, unsigned int* count) {
    char* command = malloc(strlen("service_remove") + 1);
    if (command == NULL) {
        return -1;
    }
    strcpy(command, "supervisor_list");
    if(send_command(supervisor, command, strlen(command), NULL, 0)){
        perror("send_command");
        return -1;
    }
    //Receive response from daemon
    char* response;
    ssize_t response_size;
    char* command_response;
    ssize_t command_response_size;
    if(receive_command(supervisor, &command_response, &command_response_size, &response, &response_size)){
        perror("receive_command");
        return -1;
    }
    char* currentBufferPosition = response;
    // Copy count
    memcpy(count, currentBufferPosition, sizeof(*count));
    currentBufferPosition += sizeof(*count);
    // Copy service_names
    *service_names = malloc(*count * sizeof(char *));
    if (*service_names == NULL) {
        perror("malloc");
        return -1;
    }
    for (int i = 0; i < *count; i++) {
        // Copy size of servicename
        int servicename_size = decode_length(currentBufferPosition);
        currentBufferPosition += LENGTH_SIZE;
        // Copy servicename
        (*service_names)[i] = malloc(servicename_size + 1);
        if ((*service_names)[i] == NULL) {
            perror("malloc");
            free(*service_names);
            return -1;
        }
        memcpy((*service_names)[i], currentBufferPosition, servicename_size);
        (*service_names)[i][servicename_size] = '\0';
        currentBufferPosition += servicename_size;
    }
    return 0;
}

int supervisor_freelist(supervisor_t supervisor, char** service_names, int count) {
    char* command = malloc(strlen("service_remove") + 1);
    if (command == NULL) {
        return -1;
    }
    strcpy(command, "supervisor_freelist");
    ssize_t totalBufferSize = LENGTH_SIZE;
    for (int i = 0; i < count; i++) {
        totalBufferSize += LENGTH_SIZE;
        totalBufferSize += strlen(service_names[i]);
    }
    char* buffer = malloc(totalBufferSize);
    // Copy count
    if (sprintf(buffer, "%0*zx", LENGTH_SIZE, count) < 0) {
        free(buffer);
        return -1;
    }
    char* currentBufferPosition = buffer + LENGTH_SIZE;
    // Copy service_names
    for (int i = 0; i < count; i++) {
        // Copy size of servicename
        if (sprintf(currentBufferPosition, "%0*zx", LENGTH_SIZE, strlen(service_names[i])) < 0) {
            free(buffer);
            return -1;
        }
        currentBufferPosition += LENGTH_SIZE;
        // Copy servicename
        memcpy(currentBufferPosition, service_names[i], strlen(service_names[i]));
        currentBufferPosition += strlen(service_names[i]);
    }
    if(send_command(supervisor, command, strlen(command), buffer, totalBufferSize)){
        perror("send_command");
        return -1;
    }
    //Receive response from daemon
    char* response;
    ssize_t response_size;
    char* command_response;
    ssize_t command_response_size;
    if(receive_command(supervisor, &command_response, &command_response_size, &response, &response_size)){
        perror("receive_command");
        return -1;
    }
    if(strcmp(command_response, "ok") != 0){
        printf("Received error response: %s\n", response);
        return -1;
    }
    return 0;
}