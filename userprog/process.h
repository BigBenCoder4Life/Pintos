#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

#define PROCESS_KILLED -1

/* Process identifier type.
   You can redefine this to whatever type you like. */
typedef int pid_t;
#define PID_ERROR ((pid_t) -1)          /* Error value for pid_t. */

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

#endif /* userprog/process.h */
