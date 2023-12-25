#ifndef SUPERVISORLIBRARY_SUPERVISOR_H
#define SUPERVISORLIBRARY_SUPERVISOR_H

#include <stdbool.h>
#include "service.h"



typedef struct {
    const char *servicename;
    int status;
} service_info_t;


typedef int supervisor_t;

supervisor_t supervisor_init();
int supervisor_close(supervisor_t);
void bebino(supervisor_t);

#endif //SUPERVISORLIBRARY_SUPERVISOR_H
