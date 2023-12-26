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
