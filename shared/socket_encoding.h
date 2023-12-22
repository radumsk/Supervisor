#ifndef SUPERVISORDAEMON_SOCKET_ENCODING_H
#define SUPERVISORDAEMON_SOCKET_ENCODING_H

#define LENGTH_SIZE 4
#define SOCKET_PATH "/var/run/supervisor/supervisor.sock"
#define BUFFER_SIZE 1024


#include <string.h>
#include <stdbool.h>
#include <sys/types.h>

char* encode_message(char* message, ssize_t message_size);
bool is_message_complete(char* buffer, ssize_t buffer_size);
char* decode_message(char* buffer, ssize_t buffer_size);
int send_message(int socket, char* message, ssize_t message_size);
int receive_message(int socket, char** message, ssize_t* message_size);

#endif //SUPERVISORDAEMON_SOCKET_ENCODING_H
