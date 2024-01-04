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


service_t service_create(
        supervisor_t supervisor,
        const char * servicename,
        const char * program_path,
        const char ** argv,
        int argc,
        int flags
) {
    service_t service;

    service.servicename = strdup(servicename);
    service.supervisor = supervisor;
    service.flags = flags;

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        free(service.servicename);
    }
    else if (pid > 0) {
        service.pid = pid;
        if (flags & SUPERVISOR_FLAGS_CREATESTOPPED) {
            kill(pid, SIGSTOP);
            service.status = SUPERVISOR_STATUS_PENDING;
        }
        else {
            service.status = SUPERVISOR_STATUS_RUNNING;
        }
        return service;
    }
    else {
        service.args = malloc((argc + 2) * sizeof(char*));
        service.args[0] = strdup(program_path);
        for (int i = 0; i < argc; i++) {
            service.args[i + 1] = strdup(argv[i]);
        }
        service.args[argc + 1] = NULL;
        execv(program_path, service.args);
        perror("execv");
        return service;
    }

}



int service_close(service_t service) {
    free(service.servicename);
}


service_t service_open(supervisor_t supervisor, const char *servicename) {
    service_t service;

    service.servicename = strdup(servicename);
    service.supervisor = supervisor;


    char open_command[256];
    snprintf(open_command, sizeof(open_command), "open:%s", servicename);

    // Send the open command to the supervisor
    if (send_command(supervisor, open_command, strlen(open_command), NULL, 0)) {
        perror("send_command");
        free(service.servicename);
        return service;
    }

    char *response_command;
    ssize_t response_command_size;
    void *response_params;
    ssize_t response_params_size;

    if (receive_command(supervisor, &response_command, &response_command_size, &response_params, &response_params_size)) {
        perror("receive_command");
        free(service.servicename);
        return service;
    }

    if (strcmp(response_command, "ok") != 0) {
        printf("Received unexpected command: %s\n", response_command);
        free(service.servicename);
        free(response_command);
        free(response_params);
        return service;
    }

    free(response_command);
    free(response_params);

    return service;
}

/*
int service_suspend(service_t service){

    if (send_command(service.supervisor, "suspend", 7, service.servicename, strlen(service.servicename))) {
        perror("send_command");
        return -1;
    }

    char *command;
    ssize_t command_size;
    void *params;
    ssize_t params_size;

    if (receive_command(service.supervisor, &command, &command_size, &params, &params_size)) {
        perror("receive_command");
        return -1;
    }

    if (strcmp(command, "ok") != 0) {
        printf("Received unexpected command: %s\n", command);
        return -1;
    }

    if (params_size == sizeof(int)) {
        memcpy(&service.status, params, params_size);
    } else {
        printf("Received unexpected params size for service status\n");
    }

 //   kill(service.pid, SIGSTOP); mutat in listerer.c
    free(command);
    free(params);

    return 0;
}

int service_resume(service_t service){

    if (send_command(service.supervisor, "resume", 6, service.servicename, strlen(service.servicename))) {
        perror("send_command");
        return -1;
    }

    char *command;
    ssize_t command_size;
    void *params;
    ssize_t params_size;

    if (receive_command(service.supervisor, &command, &command_size, &params, &params_size)) {
        perror("receive_command");
        return -1;
    }

    if (strcmp(command, "ok") != 0) {
        printf("Received unexpected command: %s\n", command);
        return -1;
    }

    if (params_size == sizeof(int)) {
        memcpy(&service.status, params, params_size);
    } else {
        printf("Received unexpected params size for service status\n");
    }
    // logica mutata in listener.c
    //   kill(service.pid, SIGCONT);
    free(command);
    free(params);

    return 0;
}

int service_cancel(service_t service){
    // opreste serviciul, in timp ce handlerul obtinut cu service_open() este inca valid
    // (nu a fost inchis cu service_close() sau service_remove())

    if (send_command(service.supervisor, "cancel", 6, service.servicename, strlen(service.servicename))) {
        perror("send_command");
        return -1;
    }

    char *command;
    ssize_t command_size;
    void *params;
    ssize_t params_size;

    if (receive_command(service.supervisor, &command, &command_size, &params, &params_size)) {
        perror("receive_command");
        return -1;
    }

    if (strcmp(command, "ok") != 0) {
        printf("Received unexpected command: %s\n", command);
        return -1;
    }

    if (params_size == sizeof(int)) {
        memcpy(&service.status, params, params_size);
    } else {
        printf("Received unexpected params size for service status\n");
    }
// logica mutata in listener.c
//    int result = kill(service.pid, SIGKILL);
//
//    if (result == -1) {
//        perror("kill");
//    }

    free(command);
    free(params);

}

int service_remove(service_t service){
    // inchide handlerul obtinut cu service_open() si elibereaza resursele asociate

    if (send_command(service.supervisor, "remove", 6, service.servicename, strlen(service.servicename))) {
        perror("send_command");
        return -1;
    }

    char *command;
    ssize_t command_size;
    void *params;
    ssize_t params_size;

    if (receive_command(service.supervisor, &command, &command_size, &params, &params_size)) {
        perror("receive_command");
        return -1;
    }

    if (strcmp(command, "ok") != 0) {
        printf("Received unexpected command: %s\n", command);
        return -1;
    }

    if (params_size == sizeof(int)) {
        memcpy(&service.status, params, params_size);
    } else {
        printf("Received unexpected params size for service status\n");
    }

    free(command);
    free(params);
    free(service.servicename);
    free(service.args);
    service.supervisor = -1;
    service.pid = -1;
    service.status = -1;
    service.restart_times = -1;
    service.flags = -1;
    service.service_id = -1;
}
*/
