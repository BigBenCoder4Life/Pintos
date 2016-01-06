#include <stdio.h>
#include <string.h>
#include <syscall.h>

int
main (int argc, char *argv[])
{
    int fd, ret_val = 0;
    char read_buffer[70];
    char write_buffer[] = "this was written to the test file";
    char write_buffer2[] = "A";

    // THE SECOND ARG PASSED IS test_file.txt WHICH WE SHOULD OPEN
    fd = open(argv[1]);

    if (fd > 2)
    {
       printf("\n");

       printf("filesize of fd: %d\n", filesize(fd));

       seek (fd, filesize(fd));

       ret_val = write (fd, write_buffer2, sizeof(write_buffer2));

       seek(fd, 0);

       read (fd, read_buffer, sizeof(read_buffer));

       printf("\n\n");
    }

    return ret_val;
}
