#include <stdio.h>
#include <string.h>
#include <syscall.h>

int
main (int argc, char *argv[])
{
    int fd;
    char write_buffer[] = "testing filesize 1... testing filesize 2....";
    char read_buffer[70];

    // THE SECOND ARG PASSED IS test_file.txt WHICH WE SHOULD OPEN
    fd = open(argv[1]);

    if (fd > 2)
    {
       printf("\n");

       printf("before writing the size %s is %d\n", argv[1], filesize(fd));

       seek(fd, 9);

       write (fd, write_buffer, sizeof(write_buffer));

       printf("after writing the size %s is %d\n", argv[1], filesize(fd));

       seek(fd, 0);

       read (fd, read_buffer, sizeof(read_buffer));

       printf("\n\n");
    }

    return 0;
}
