#include <stdio.h>
#include <string.h>
#include <syscall.h>

int
main (int argc, char *argv[])
{
    halt();

    printf("This shouldn't be read!!!\n");

    return 0;
}
