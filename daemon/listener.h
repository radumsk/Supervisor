#ifndef SUPERVISORDAEMON_LISTENER_H
#define SUPERVISORDAEMON_LISTENER_H

struct listen_return_t {
    int error;
};

struct listen_params_t {
    int client_socket;
};

void* listen_on_socket(void* params);

#endif //SUPERVISORDAEMON_LISTENER_H
