/*pass must be terminated with -1 exit code. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"
#include <stdio.h>

int
main (void)
{
  int ret = open ((char *) 0x20101234);
  
  if (ret >= 0)
    printf("should have called exit(-1) but instead returned %d\n", ret);

  return 0;
}

