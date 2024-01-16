#include "../library/supervisor.h"
#include <stdio.h>

int main(){
    supervisor_t supervisor = supervisor_init();
    printf("Supervisor initialized with fd: %d\n", supervisor);

    service_t service = service_create(supervisor, "bebino", "./executabileTest/test2", NULL, 0, 0);
    service_t foundService = service_open(supervisor, "bebino");
    printf("Service found: %d\n", foundService);
    int status = service_status(supervisor, service);
    printf("Service status: %d\n", status);

    // service_t service2 = service_create(supervisor, "bebino2", "./executabileTest/test3", NULL, 0, 0);
    // int status2 = service_status(supervisor, service2);
    // printf("Service status: %d\n", status2);

    unsigned int count;
    char** service_names;
    supervisor_list(supervisor, &service_names, &count);
    // Print service names
    for(int i = 0; i < count; i++){
        printf("Service name: %s\n", service_names[i]);
    }

    supervisor_freelist(supervisor, service_names, count);


    // service_close(supervisor, service);
    // service_close(supervisor, service2);

    supervisor_close(supervisor);
    return 0;
}