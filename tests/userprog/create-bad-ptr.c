/* Passes a bad pointer to the create system call,
   which must either case the process to be terminated with
   exit code -1 or must return false from the create system
   call, exiting with code 0. */

#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  msg ("create(0x20101234): %d", create ((char *) 0x20101234, 0));
}
