#include <stdio.h>
#include <string.h>
#include <syscall.h>

int
main (int argc, char *argv[])
{
    int fd, ret_val = 0;
    char buffer[300];

    // THE SECOND ARG PASSED IS test_file.txt WHICH WE SHOULD OPEN
    fd = open(argv[1]);

    printf("filesize is: %d\n", filesize(fd));

    if (fd > 2)
    {
       ret_val = read (fd, buffer, 300);
    }

    return ret_val;
}
