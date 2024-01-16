#ifndef SUPERVISORLIBRARY_METHODS_H
#define SUPERVISORLIBRARY_METHODS_H

#include "../shared/service.h"

struct bebino_t{
    int x;
    char y;
};

struct service_info_t{
    supervisor_t supervisor;
    char * servicename;
    //int service_id;
    int pid;
    char ** args;
    int argc;
    int status;
    int restart_times;
    int flags;
};


void print_bebino(int socket, struct bebino_t* bebino);

struct service_create_args_t* deserialize_service_create_args(char* buffer, ssize_t params_size);


char* serialize_service_create_args(struct service_create_args_t args, int* size);

//cerinta 2
service_t service_create(
        const char *servicename,
        const char *program_path,
        const char **argv,
        int argc,
        int flags
);
int service_close(service_t service);

//cerinta 3
service_t service_open(
        const char *servicename
);
int service_status(service_t service);

//cerina 4
int service_suspend(service_t service);
int service_resume(service_t service);

//cerinta 5
int service_cancel(service_t service);
int service_remove(service_t service);

//cerinta 6

service_t service_restart(const char *servicename, const char *program_path, const char **argv, int argc, int flags);

int supervisor_list(
        char ***service_names,
        unsigned int *count
);

int supervisor_freelist(char **service_names, int count);

#endif //SUPERVISORLIBRARY_METHODS_H
