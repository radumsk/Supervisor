#ifndef SERVICE_H
#define SERVICE_H

#define SUPERVISOR_FLAGS_CREATESTOPPED 0x1
#define SUPERVISOR_FLAGS_RESTARTTIMES(times) ((times & 0xF) << 16)
#define SUPERVISOR_STATUS_RUNNING 0x1
#define SUPERVISOR_STATUS_PENDING 0x2
#define SUPERVISOR_STATUS_STOPPED 0x4
#define MAX_SERVICES 100
#define MAX_RESTART_TIMES 3

typedef int supervisor_t;

typedef int service_t;

struct service_create_args_t {
    supervisor_t supervisor;
    char *servicename;
    char *program_path;
    char **argv;
    int argc;
    int flags;
};


#endif //SERVICE_H
