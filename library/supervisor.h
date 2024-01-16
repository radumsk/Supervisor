#ifndef SUPERVISORLIBRARY_SUPERVISOR_H
#define SUPERVISORLIBRARY_SUPERVISOR_H

#include <stdbool.h>
#include "service.h"
char* serialize_service_create_args(struct service_create_args_t args, int* size);

//cerinta 2
service_t service_create(
        supervisor_t supervisor,
        const char *servicename,
        const char *program_path,
        const char **argv,
        int argc,
        int flags
);
int service_close(supervisor_t supervisor, service_t service);

//cerinta 3
service_t service_open(
        supervisor_t supervisor,
        const char *servicename
);
int service_status(supervisor_t supervisor, service_t service);

//cerina 4
int service_suspend(supervisor_t supervisor, service_t service);
int service_resume(supervisor_t supervisor, service_t service);

//cerinta 5
int service_cancel(supervisor_t supervisor, service_t service);
int service_remove(supervisor_t supervisor, service_t service);

//cerinta 6

service_t service_restart(supervisor_t supervisor, const char *servicename, const char *program_path, const char **argv, int argc, int flags);

int supervisor_list(
        supervisor_t supervisor,
        char ***service_names,
        unsigned int *count
);

int supervisor_freelist(
        supervisor_t supervisor,
        char **service_names,
        int count
);


supervisor_t supervisor_init();
int supervisor_close(supervisor_t);
void bebino(supervisor_t);



#endif //SUPERVISORLIBRARY_SUPERVISOR_H
