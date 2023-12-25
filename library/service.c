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
        const char *servicename,
        const char *program_path,
        const char **argv,
        int argc,
        int flags
) {
    service_t service;

    service.servicename = strdup(servicename);
    service.supervisor = supervisor;
    service.flags = flags;
    service.restart_times = (flags >> 16) & 0xF;

    pid_t pid;
    int restart_count = 0;

    do {
        pid = fork();
        if (pid < 0) {
            perror("fork");
            free(service.servicename);
            // Handle error
        } else if (pid > 0) {
            // Parent process
            service.pid = pid;
            if (flags & SUPERVISOR_FLAGS_CREATESTOPPED) {
                kill(pid, SIGSTOP);
                service.status = SUPERVISOR_STATUS_PENDING;
            } else {
                service.status = SUPERVISOR_STATUS_RUNNING;
            }
            return service;
        } else {
            // Child process
            service.args = malloc((argc + 2) * sizeof(char *));
            service.args[0] = strdup(program_path);
            for (int i = 0; i < argc; i++) {
                service.args[i + 1] = strdup(argv[i]);
            }
            service.args[argc + 1] = NULL;
            execv(program_path, service.args);
            perror("execv");
            restart_count++;
            if (restart_count > service.restart_times) {
                service.status = SUPERVISOR_STATUS_STOPPED;
                free(service.args);
            } else {
                sleep(2);
            }
        }
    } while (restart_count <= service.restart_times);

    return service;
}



int service_close(service_t service) {
    free(service.servicename);
}


service_t service_open(supervisor_t supervisor, const char *servicename) {
    service_t service;
    service.servicename = strdup(servicename);
    service.supervisor = supervisor;

    if (send_command(supervisor, "open", 4, servicename, strlen(servicename))) {
        perror("send_command");
        return service;
    }

    char *command;
    ssize_t command_size;
    void *params;
    ssize_t params_size;

    if (receive_command(supervisor, &command, &command_size, &params, &params_size)) {
        perror("receive_command");
        return service;
    }

    if (strcmp(command, "ok") != 0) {
        printf("Received unexpected command: %s\n", command);
        return service;
    }

    if (params_size == sizeof(int)) {
        memcpy(&service.status, params, params_size);
    } else {
        printf("Received unexpected params size for service status\n");
    }

    free(command);
    free(params);

    return service;
}

int service_status(service_t service) {
    return service.status;
}

