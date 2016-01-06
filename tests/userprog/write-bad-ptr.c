/* Passes an invalid pointer to the write system call.
   The process must either be terminated with exit
   code -1 or must return -1 from the write system
   call, exiting with code 0. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  int handle;
  int result;
  CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");

  result = write (handle, (char *) 0x10123420, 123);
  if (result >= 0)
    fail ("should have exited with -1");
}
