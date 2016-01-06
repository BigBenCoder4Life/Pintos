#include <stdio.h>
#include <string.h>
#include <syscall.h>

int
main (int argc, char *argv[])
{
    int fd;
    char read_buffer[50];
    char write_buffer[] = "this was appended to the test file";

    // THE SECOND ARG PASSED IS test_file.txt WHICH WE SHOULD OPEN
    fd = open(argv[1]);

    if (fd > 2)
    {
       printf("\n");

       seek(fd, 10);

       write (fd, write_buffer, sizeof(write_buffer));

       seek(fd, 0);

       read (fd, read_buffer, 50);

       printf("\n\n");
    }

    return 0;
}
