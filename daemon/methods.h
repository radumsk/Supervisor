#ifndef SUPERVISORLIBRARY_METHODS_H
#define SUPERVISORLIBRARY_METHODS_H


struct bebino_t{
    int x;
    char y;
};

void print_bebino(int socket, struct bebino_t* bebino);

#endif //SUPERVISORLIBRARY_METHODS_H
