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

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;



service_t service_create(
        supervisor_t supervisor,
        const char * servicename,
        const char * program_path,
        const char ** argv,
        int argc,
        int flags
) {

    service_t new_service_id = -1;

    for(int i = 0; i < LAST_INDEX; i++){
        if(SERVICES[i].status == 0){
            new_service_id = i;
            break;
        }
    }

    pthread_mutex_lock(&mutex); 

    SERVICES[new_service_id].servicename = strdup(servicename);
    SERVICES[new_service_id].supervisor = supervisor;
    SERVICES[new_service_id].flags = flags;

    
    if(new_service_id == -1)
        new_service_id = LAST_INDEX++;
    

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        free(SERVICES[new_service_id].servicename);
    }
    else if (pid > 0) {
        SERVICES[new_service_id].pid = pid;
        if (flags & SUPERVISOR_FLAGS_CREATESTOPPED) {
            kill(pid, SIGSTOP);
            SERVICES[new_service_id].status = SUPERVISOR_STATUS_PENDING;
        }
        else {
            SERVICES[new_service_id].status = SUPERVISOR_STATUS_RUNNING;
        }
        pthread_mutex_unlock(&mutex); 
        return new_service_id;
    }
    else {
        pthread_mutex_unlock(&mutex);  

        SERVICES[new_service_id].args = malloc((argc + 2) * sizeof(char*));
        SERVICES[new_service_id].args[0] = strdup(program_path);
        for (int i = 0; i < argc; i++) {
            SERVICES[new_service_id].args[i + 1] = strdup(argv[i]);
        }
        SERVICES[new_service_id].args[argc + 1] = NULL;
        execv(program_path, SERVICES[new_service_id].args);
        perror("execv");
        return new_service_id;
    }
}



int service_close(service_t service) {
    SERVICES[service].status = 0;
    free(SERVICES[service].servicename);
    SERVICES[service].supervisor = 0;
    SERVICES[service].pid = -1;
    free(SERVICES[service].args);
    SERVICES[service].restart_times = 0;
    SERVICES[service].flags = 0;
}


service_t service_open(supervisor_t supervisor, const char *servicename) {
    for(int i=0; i < LAST_INDEX; i++){
        if(strcmp(SERVICES[i].servicename, servicename) == 0){
            return i;
        }
    }
    return -1;
}

int service_status(service_t service){
    return SERVICES[service].status;
}


int service_suspend(service_t service){
    if(SERVICES[service].status) {
        kill(SERVICES[service].pid, SIGSTOP);
        SERVICES[service].status = SUPERVISOR_STATUS_PENDING;
        return SERVICES[service].status;
    }
    else{
        return -1;
    }
}

int service_resume(service_t service){
    if(SERVICES[service].status) {
        kill(SERVICES[service].pid, SIGCONT);
        SERVICES[service].status = SUPERVISOR_STATUS_RUNNING;
        return SERVICES[service].status;
    }
    else{
        return -1;
    }
}

int service_cancel(service_t service){
    if(SERVICES[service].status) {
        kill(SERVICES[service].pid, SIGKILL);
        SERVICES[service].status = SUPERVISOR_STATUS_STOPPED;
        return SERVICES[service].status;
    }
    else{
        return -1;
    }

}

int service_remove(service_t service){
    if(SERVICES[service].status) {
        service_cancel(service);
        service_close(service);
        return 0;
    }
    else{
        return -1;
    }
}


/*
int service_suspend(service_t service){

    if (send_command(SERVICES[new_service_id].supervisor, "suspend", 7, SERVICES[new_service_id].servicename, strlen(SERVICES[new_service_id].servicename))) {
        perror("send_command");
        return -1;
    }

    char *command;
    ssize_t command_size;
    void *params;
    ssize_t params_size;

    if (receive_command(SERVICES[new_service_id].supervisor, &command, &command_size, &params, &params_size)) {
        perror("receive_command");
        return -1;
    }

    if (strcmp(command, "ok") != 0) {
        printf("Received unexpected command: %s\n", command);
        return -1;
    }

    if (params_size == sizeof(int)) {
        memcpy(&SERVICES[new_service_id].status, params, params_size);
    } else {
        printf("Received unexpected params size for service status\n");
    }

    kill(SERVICES[new_service_id].pid, SIGSTOP);
    free(command);
    free(params);

    return 0;
}

int service_resume(service_t service){

    if (send_command(SERVICES[new_service_id].supervisor, "resume", 6, SERVICES[new_service_id].servicename, strlen(SERVICES[new_service_id].servicename))) {
        perror("send_command");
        return -1;
    }

    char *command;
    ssize_t command_size;
    void *params;
    ssize_t params_size;

    if (receive_command(SERVICES[new_service_id].supervisor, &command, &command_size, &params, &params_size)) {
        perror("receive_command");
        return -1;
    }

    if (strcmp(command, "ok") != 0) {
        printf("Received unexpected command: %s\n", command);
        return -1;
    }

    if (params_size == sizeof(int)) {
        memcpy(&SERVICES[new_service_id].status, params, params_size);
    } else {
        printf("Received unexpected params size for service status\n");
    }

    kill(SERVICES[new_service_id].pid, SIGCONT);
    free(command);
    free(params);

    return 0;
}

int service_cancel(service_t service){
    // opreste serviciul, in timp ce handlerul obtinut cu service_open() este inca valid
    // (nu a fost inchis cu service_close() sau service_remove())

    if (send_command(SERVICES[new_service_id].supervisor, "cancel", 6, SERVICES[new_service_id].servicename, strlen(SERVICES[new_service_id].servicename))) {
        perror("send_command");
        return -1;
    }

    char *command;
    ssize_t command_size;
    void *params;
    ssize_t params_size;

    if (receive_command(SERVICES[new_service_id].supervisor, &command, &command_size, &params, &params_size)) {
        perror("receive_command");
        return -1;
    }

    if (strcmp(command, "ok") != 0) {
        printf("Received unexpected command: %s\n", command);
        return -1;
    }

    if (params_size == sizeof(int)) {
        memcpy(&SERVICES[new_service_id].status, params, params_size);
    } else {
        printf("Received unexpected params size for service status\n");
    }

    int result = kill(SERVICES[new_service_id].pid, SIGKILL);

    if (result == -1) {
        perror("kill");
    }

    free(command);
    free(params);

}

int service_remove(service_t service){
    // inchide handlerul obtinut cu service_open() si elibereaza resursele asociate

    if (send_command(SERVICES[new_service_id].supervisor, "remove", 6, SERVICES[new_service_id].servicename, strlen(SERVICES[new_service_id].servicename))) {
        perror("send_command");
        return -1;
    }

    char *command;
    ssize_t command_size;
    void *params;
    ssize_t params_size;

    if (receive_command(SERVICES[new_service_id].supervisor, &command, &command_size, &params, &params_size)) {
        perror("receive_command");
        return -1;
    }

    if (strcmp(command, "ok") != 0) {
        printf("Received unexpected command: %s\n", command);
        return -1;
    }

    if (params_size == sizeof(int)) {
        memcpy(&SERVICES[new_service_id].status, params, params_size);
    } else {
        printf("Received unexpected params size for service status\n");
    }

    free(command);
    free(params);
    free(SERVICES[new_service_id].servicename);
    free(SERVICES[new_service_id].args);
    SERVICES[new_service_id].supervisor = -1;
    SERVICES[new_service_id].pid = -1;
    SERVICES[new_service_id].status = -1;
    SERVICES[new_service_id].restart_times = -1;
    SERVICES[new_service_id].flags = -1;
    SERVICES[new_service_id].service_id = -1;
}

*/