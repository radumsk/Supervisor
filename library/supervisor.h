#ifndef SUPERVISORLIBRARY_SUPERVISOR_H
#define SUPERVISORLIBRARY_SUPERVISOR_H

#include <stdbool.h>
#include "service.h"


/*
typedef struct {
    const char *servicename;
    pid_t pid;
    int status;
    int restart_times;
    supervisor_t supervisor;
} service_info_t;
*/

supervisor_t supervisor_init();
int supervisor_close(supervisor_t);
void bebino(supervisor_t);



#endif //SUPERVISORLIBRARY_SUPERVISOR_H
