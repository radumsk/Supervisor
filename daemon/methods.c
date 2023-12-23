#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "methods.h"
#include "../library/supervisor.h"
#include "../shared/socket_encoding.h"

extern pthread_mutex_t mutex;

void print_bebino(int socket, struct bebino_t* bebino){
    pthread_mutex_lock(&mutex);
    printf("Bebino: %d, %c\n", bebino->x, bebino->y);
    struct bebino_t bebino_response;
    bebino_response.x = 10;
    bebino_response.y = 'b';
    if(send_command(socket, "ok", 2, &bebino_response, sizeof(struct bebino_t))){
        perror("send_command");
        return;
    }
    pthread_mutex_unlock(&mutex);
}