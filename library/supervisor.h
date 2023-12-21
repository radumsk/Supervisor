#ifndef SUPERVISORLIBRARY_SUPERVISOR_H
#define SUPERVISORLIBRARY_SUPERVISOR_H

#include <stdbool.h>

typedef int supervisor_t;

supervisor_t supervisor_init();
int supervisor_close(supervisor_t);
void bebino(supervisor_t);

#endif //SUPERVISORLIBRARY_SUPERVISOR_H
