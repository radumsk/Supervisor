#include <pthread.h>
#include "methods.h"
#include "../library/supervisor.h"
#include "../shared/socket_encoding.h"

extern pthread_mutex_t mutex;

void print_bebino(int socket){
    pthread_mutex_lock(&mutex);
    send_message(socket, "bebino", 6);
    pthread_mutex_unlock(&mutex);
}