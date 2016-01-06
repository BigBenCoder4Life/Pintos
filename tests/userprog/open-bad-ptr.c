/* Passes an invalid pointer to the open system call.
   The process must be terminated with -1 exit code. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  int ret = open ((char *) 0x20101234);
  msg ("open(0x20101234): %d", ret);
  if (ret >= 0)
    fail ("should have called exit(-1)");
}
