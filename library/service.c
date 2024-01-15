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
        free(params);
        return -1;
    }

    if(send_command(supervisor, command, strlen(command), params, params_size)){
        perror("send_command");
        free(params);
        return -1;
    }

    //Receive response from daemon

    service_t* response;
    ssize_t response_size;
    char* command_response;
    ssize_t command_response_size;

    if(receive_command(supervisor, &command_response, &command_response_size, &response, &response_size)){
        perror("receive_command");
        free(params);
        return -1;
    }

    if(strcmp(command_response, "ok") != 0){
        printf("Received unexpected response: %s\n", command_response);
        free(params);
        return -1;
    }

    return *response;
}

int service_close(service_t service) {

}