#include <stdio.h>
#include <unistd.h>

int main()
{
    printf("Start cicilu infinit cu sleep 5 secunde\n");
    while(1)
    {
        sleep(5);
    }
    printf("Cumva a ieșit din ciclul infinit\n");
    return 0;
}
