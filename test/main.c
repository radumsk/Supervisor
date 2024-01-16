#include "../library/supervisor.h"
#include <stdio.h>
#include <unistd.h>

int main(){
    supervisor_t supervisor = supervisor_init();
    printf("Supervisor initialized with fd: %d\n", supervisor);

    service_t service = service_create(supervisor, "bebino", "../executabileTest/test2", NULL, 0, 0);
    //service_t foundService = service_open(supervisor, "bebino");
    //printf("Service found: %d\n", foundService);
    int status = service_status(supervisor, service);
    printf("Service status: %d\n", status);

     service_t service2 = service_create(supervisor, "bebino2", "../executabileTest/test3", NULL, 0, 0);
     int status2 = service_status(supervisor, service2);
     printf("Service status: %d\n", status2);


    service_t service3 = service_create(supervisor, "bebino3", "../executabileTest/test3", NULL, 0, 0);
    //service_t foundService3 = service_open(supervisor, "bebino3");
    int status3 = service_status(supervisor, service3);
    printf("Service status: %d\n", status3);

    service_t service4 = service_create(supervisor, "bebino4", "../executabileTest/test4", NULL, 0, 0);
    int status4 = service_status(supervisor, service4);
    sleep(10);
    service_t foundService4 = service_open(supervisor, "bebino4");
    status4 = service_status(supervisor, service4);
    printf("Service status: %d\n", status4);
//
//    service_t service5 = service_create(supervisor, "bebino5", "../executabileTest/test5", NULL, 0, 0);
//    int status5 = service_status(supervisor, service5);
//    printf("Service status: %d\n", status5);
//
//    service_t service6 = service_create(supervisor, "bebino6", "../executabileTest/test6", NULL, 0, 0);
//    int status6 = service_status(supervisor, service6);
//    printf("Service status: %d\n", status6);
//
//    service_t service7 = service_create(supervisor, "bebino7", "../executabileTest/test7", NULL, 0, 0);
//    int status7 = service_status(supervisor, service7);
//    printf("Service status: %d\n", status7);

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