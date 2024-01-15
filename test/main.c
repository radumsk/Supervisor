#include "../library/supervisor.h"
#include <stdio.h>

int main(){
    supervisor_t supervisor = supervisor_init();
    printf("Supervisor initialized with fd: %d\n", supervisor);
    service_t service = service_create(supervisor, "bebino", "./test/bebino", NULL, 0, 0);
    service_close(service);
    supervisor_close(supervisor);
    return 0;
}