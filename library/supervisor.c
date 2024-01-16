#include "supervisor.h"
#include "../shared/socket_encoding.h"


#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/un.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

supervisor_t supervisor_init(){
    supervisor_t supervisor = socket(AF_UNIX, SOCK_STREAM, 0);

    if(supervisor == -1){
        perror("socket");
        return -1;
    }

    struct sockaddr_un server_addr;

    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCKET_PATH);
    int slen = sizeof(server_addr);

    if(connect(supervisor, (struct sockaddr *) &server_addr, slen) == -1){
        perror("connect");
        return -1;
    }

    printf("Connected to supervisor\n");

    return supervisor;
}

int supervisor_close(supervisor_t supervisor){
    char* commandBuffer = malloc(4);
    strcpy(commandBuffer, "stop");
    if(send_command(supervisor, commandBuffer, 4, NULL, 0)){
        perror("send_message");
        return errno;
    }
    char* message;
    ssize_t message_size;
    if(receive_message(supervisor, &message, &message_size)){
        perror("receive_message");
        return errno;
    }
    if(strncmp(message, "ok", message_size) != 0){
        printf("Received unexpected message: %s\n", message);
        return -1;
    }
    if(close(supervisor)){
        perror("close");
        return errno;
    }
    return 0;
}



struct bebino_t{
    int x;
    char y;
};

void bebino(supervisor_t supervisor){
    struct bebino_t bebino;
    bebino.x = 5;
    bebino.y = 'a';
    if(send_command(supervisor, "bebino", 6, &bebino, sizeof(struct bebino_t))){
        perror("send_command");
        return;
    }
    char* command;
    ssize_t command_size;
    void* params;
    ssize_t params_size;
    if(receive_command(supervisor, &command, &command_size, &params, &params_size)){
        perror("receive_command");
        return;
    }
    if(strncmp(command, "ok", command_size) != 0){
        printf("Received unexpected command: %s\n", command);
        return;
    }
    struct bebino_t* bebino_response = (struct bebino_t*) params;
    printf("Received bebino response: %d, %c\n", bebino_response->x, bebino_response->y);

    free(command);
    free(params);
}