#include <stdio.h>
#include <string.h>
#include <syscall.h>

int
main (int argc, char *argv[])
{
    int fd;
    char write_buffer[] = "testing filesize 1... testing filesize 2....";

    // THE SECOND ARG PASSED IS test_file.txt WHICH WE SHOULD OPEN
    fd = open(argv[1]);

    printf("The fd is: %d\n", fd);

    if (fd > 2)
    {
       printf("\n");

       seek(fd, 5);

       printf("before writing to: %s \tthe position is: %d\n", argv[1], tell(fd));

       write (fd, write_buffer, sizeof(write_buffer));

       seek (fd, 15);

       printf("before writing to: %s \tthe position is: %d\n", argv[1], tell(fd));

       printf("\n\n");
    }

    return 0;
}
