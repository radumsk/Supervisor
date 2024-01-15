//buufer overflow
#include <stdio.h>
#include "string.h"

int main()
{
    char buffer[5];
    printf("buffer overflow\n");
    strcpy(buffer, "0123456789ABCDEF");
    return 0;
}