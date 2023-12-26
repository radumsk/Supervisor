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



service_info_t services[MAX_SERVICES];

void add_service_to_array(service_info_t* service) {
    for (int i = 0; i < MAX_SERVICES; ++i) {
        if (services[i].pid == 0) {
            services[i] = *service;
            break;
        }
    }
}

void handle_child_exit(int signo) {
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < MAX_SERVICES; ++i) {
            if (services[i].pid == pid) {
                services[i].status = determine_service_status(status);

                if (services[i].status == SUPERVISOR_STATUS_STOPPED && services[i].restart_times > 0) {
                    service_t service = service_create(services[i].supervisor, services[i].servicename, "sample_service", NULL, 0, SUPERVISOR_FLAGS_RESTARTTIMES(services[i].restart_times));

                    if (service.status == SUPERVISOR_STATUS_RUNNING) {
                        services[i].pid = service.pid;
                        services[i].status = SUPERVISOR_STATUS_RUNNING;
                    }
                }
                break;
            }
        }
    }
}

void add_service_and_open(supervisor_t supervisor, const char *servicename) {
    service_info_t new_service = {
            .pid = 0,
            .status = 0,
            .restart_times = 3

    };

    service_t service = service_create(supervisor, servicename, "sample_service", NULL, 0, SUPERVISOR_FLAGS_RESTARTTIMES(new_service.restart_times));

    if (service.status == SUPERVISOR_STATUS_RUNNING) {

        new_service.pid = service.pid;
        new_service.status = SUPERVISOR_STATUS_RUNNING;
        add_service_to_array(&new_service);
        service_open_response(supervisor, servicename, 1);
    } else {
        service_open_response(supervisor, servicename, 0);
    }
}

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
    if(send_command(supervisor, "stop", 4, NULL, 0)){
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
    if(strcmp(command, "ok") != 0){
        printf("Received unexpected command: %s\n", command);
        return;
    }
    struct bebino_t* bebino_response = (struct bebino_t*) params;
    printf("Received bebino response: %d, %c\n", bebino_response->x, bebino_response->y);

    free(command);
    free(params);
}

int service_open_response(supervisor_t supervisor, const char *servicename, int status) {
    char response_command[256];
    snprintf(response_command, sizeof(response_command), "%s:%d", servicename, status);

    send_command(supervisor, response_command, strlen(response_command), NULL, 0);

    return 0;
}

int main() {
    supervisor_t supervisor = supervisor_init();

    if (supervisor == -1) {
        perror("supervisor_init");
        return 1;
    }

    struct sigaction sa;
    sa.sa_handler = handle_child_exit;
    sigaction(SIGCHLD, &sa, NULL);

    while (1) {
        char *response_command;
        ssize_t response_command_size;
        void *response_params;
        ssize_t response_params_size;

        if (receive_command(supervisor, &response_command, &response_command_size, &response_params, &response_params_size)) {
            perror("receive_command");
            break;
        }

        if (strcmp(response_command, "service_open") == 0) {
            const char *servicename = "sample_service";
            int status = 1;  // 1 for success, 0 for failure

            if (status == 0) {
                service_t service = service_create(supervisor, servicename, "sample_service", NULL, 0, SUPERVISOR_FLAGS_RESTARTTIMES(3));

                if (service.status == SUPERVISOR_STATUS_RUNNING) {
                    service_open_response(supervisor, servicename, 1);

                    // Add the new service to the services array
                    service_info_t new_service = {
                            .pid = service.pid,
                            .status = SUPERVISOR_STATUS_RUNNING,
                            .restart_times = 3,
                    };
                    add_service_to_array(&new_service);
                } else {
                    service_open_response(supervisor, servicename, 0);
                }
            } else {
                service_open_response(supervisor, servicename, status);
            }
        }

        free(response_command);
        free(response_params);
    }

    supervisor_close(supervisor);

    return 0;
}