#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <libc.h>
#include <sys/un.h>
#include <stdbool.h>
#include <pthread.h>

#include "daemon_flags.h"
#include "listener.h"
#include "../shared/socket_encoding.h"

pthread_mutex_t mutex;

int create_socket(int* server_socket, bool force){
    // Check if socket already exists
    if(access(SOCKET_PATH, F_OK) != -1){
        // Socket already exists
        if(force){
            // Remove the socket
            if(remove(SOCKET_PATH)){
                perror("remove");
                return errno;
            }
        } else {
            printf("Socket already exists\n");
            return -1;
        }
    }

    *server_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    if(*server_socket == -1){
        perror("socket");
        return errno;
    }

    struct sockaddr_un server_addr;

    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCKET_PATH);
    int slen = sizeof(server_addr);

    if(bind(*server_socket, (struct sockaddr *) &server_addr, slen) == -1){
        perror("bind");
        return errno;
    }

    // Init mutex
    if(pthread_mutex_init(&mutex, NULL)){
        perror("pthread_mutex_init");
        return errno;
    }

    return 0;
}

int start_listening(bool force){
    int server_socket;
    int error = create_socket(&server_socket, force);
    if(error){
        return error;
    }

    if(listen(server_socket, MAX_CONNECTIONS) == -1){
        perror("listen");
        return errno;
    }
    printf("Listening on socket...\n");

    while(true){
        printf("Waiting for connection...\n");
        int client_socket = accept(server_socket, NULL, NULL);
        if(client_socket == -1){
            perror("accept");
            return errno;
        }
        printf("Connection accepted\n");
        pthread_t thread;
        struct listen_params_t *params = malloc(sizeof(struct listen_params_t));
        params->client_socket = client_socket;
        if(pthread_create(&thread, NULL, listen_on_socket, params)){
            perror("pthread_create");
            return errno;
        }
        pthread_detach(thread);
    }

    return 0;
}

int main(int argc, char** argv){
    bool daemonize = false;
    bool force = false;
    // Parse arguments
    if(argc > 1) {
        // Check if the first character of the first argument is a dash
        if(argv[1][0] != '-'){
            printf("Invalid argument: %s\n", argv[1]);
            return -1;
        }
        char* arg_char = argv[1] + 1;
        while(*arg_char != '\0'){
            switch(*arg_char){
                case 'd':
                    daemonize = true;
                    break;
                case 'f':
                    force = true;
                    break;
                default:
                    printf("Invalid argument: %c\n", *arg_char);
                    return -1;
            }
            arg_char++;
        }
    }

    if(daemonize){
        // Start the daemon
        int pid = daemon(0, 0);
        if (pid == -1) {
            perror("daemon");
            return errno;
        }
        printf("Daemon started with pid %d\n", pid);
    }

    return start_listening(force);
}