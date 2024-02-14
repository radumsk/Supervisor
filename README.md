# Supervisor

Daemon and supervisor library for UNIX environment built using sockets. It supports executing and monitoring multiple services.

## Functionalities
* Initialize and close communication with the supervisor: 
```
supervisor_t supervisor_init();
int supervisor_close(supervisor_t);
```
* Create new service, close existing service: 
```
#define SUPERVISOR_FLAGS_CREATESTOPPED 0x1
#define SUPERVISOR_FLAGS_RESTARTTIMES(times) ((times & 0xF) << 16)
service_t service_create(
  supervisor_t supervisor,
  const char * servicename,
  const char * program_path,
  const char ** argv,
  int argc,
  int flags
);
int service_close(service_t service);
```

* Update status of a service, get status: 
```
service_t service_open(
  supervisor_t supervisor,
  const char * servicename
);

#define SUPERVISOR_STATUS_RUNNING 0x1
#define SUPERVISOR_STATUS_PENDING 0x2
#define SUPERVISOR_STATUS_STOPPED 0x4
int service_status(service_t service);

```
* Suspend/reusume service: 

```
int service_suspend(service_t service);
int service_resume(service_t service);

```

* Cancel remove service:
```
int service_cancel(service_t service);
int service_remove(service_t service);

```

* List processes:
```

int supervisor_list(
  supervisor_t supervisor,
  char *** service_names,
  unsigned int * count
);

int supervisor_freelist(
  char ** service_names,
  int count
);
```
