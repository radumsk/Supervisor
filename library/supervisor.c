#include "supervisor.h"
#include "../shared/socket_encoding.h"

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/un.h>
#include <stdbool.h>
#include <sys/socket.h>

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
    if(send_message(supervisor, "stop", 4)){
        perror("send_message");
        return errno;
    }
    char* message;
    ssize_t message_size;
    if(receive_message(supervisor, &message, &message_size)){
        perror("receive_message");
        return errno;
    }
    if(strcmp(message, "ok") != 0){
        printf("Received unexpected message: %s\n", message);
        return -1;
    }
    if(close(supervisor)){
        perror("close");
        return errno;
    }
    return 0;
}

void bebino(supervisor_t supervisor){
    if(send_message(supervisor, "bebino", 6)){
        perror("send_message");
        return;
    }
    char* message;
    ssize_t message_size;
    if(receive_message(supervisor, &message, &message_size)){
        perror("receive_message");
        return;
    }
    printf("%s\n", message);
}