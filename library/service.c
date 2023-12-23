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

