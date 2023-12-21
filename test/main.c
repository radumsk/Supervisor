#include "../library/supervisor.h"
#include <stdio.h>

int main(){
    supervisor_t supervisor = supervisor_init();
    printf("Supervisor initialized with fd: %d\n", supervisor);
    bebino(supervisor);
    supervisor_close(supervisor);
    return 0;
}