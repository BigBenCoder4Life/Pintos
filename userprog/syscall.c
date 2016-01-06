#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/input.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "filesys/inode.h"
#include "threads/init.h"
#include "devices/shutdown.h"
#include "userprog/process.h"


static void syscall_handler (struct intr_frame *);
void parse_syscall_args (uint32_t *sp, int num_args, ...);
void exit   (int status);
int  read   (int fd, void *buffer, unsigned size);
int  write  (int fd, void *buffer, unsigned size);
int  open   (const char *file_name);
void seek   (int fd, int position);
int  filesize (int fd);
bool has_fd (int fd);
void halt   (void);
int  tell   (int fd);
void close  (int fd);
static int get_user (const uint8_t *uaddr);
bool create (const char *file, unsigned size);
pid_t exec (const char *cmd_line);
int wait(pid_t pid);

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/* Helper function for processing a variable number of
 arguments passed on the stack.  This function demonstrates
 the C99 variable number of arguments feature (not in the
 original C specification).  It can be called like this:

 parse_syscall_args (sp, 1, &arg1);
 parse_syscall_args (sp, 2, &arg1, &arg2);

 The parameters arg1, arg2, etc., must be uint32_t. */
void
parse_syscall_args (uint32_t *sp, int num_args, ...)
{
  va_list arg;
  int i;

  va_start (arg, num_args);
  for (i = 0; i < num_args; i++)
  *(va_arg (arg, uint32_t*)) = *++sp;
  va_end (arg);

  if ((uint32_t) sp >= (uint32_t) PHYS_BASE)
    thread_exit ();
}

/*
 *  This function with determine all system calls and
 *  send functionality to appropriate functions
 */
static void
syscall_handler (struct intr_frame *f UNUSED)
{
  // ERROR CHECKING STACK POINTER
  if (f->esp >= PHYS_BASE || f->esp <= 0x08048000 || f->esp == NULL)
    exit(-1);

  int syscall = *(int*) f->esp;

  int fd, position, status;
  char* file;
  void *buffer;
  unsigned size;
  pid_t pid;

  switch (syscall)
  {
      case (SYS_HALT):
	  halt();
	  break;
      case (SYS_EXIT):

          // ERROR CHECKING sc-bad-arg.c
          if ((f->esp + sizeof(status)) >= PHYS_BASE)
	  {
 	     f->eax = -1;
 	     exit(-1);
 	     break;
	  }

	  parse_syscall_args ((uint32_t*) f->esp, 1, &status);
	  f->eax = status;
	  exit(status);
	  break;
      case (SYS_EXEC):
	  parse_syscall_args((uint32_t*) f->esp, 1, &file);
	  if (file >= PHYS_BASE || file <= 0x08048000)
	    f->eax = -1;
	  else if (pagedir_get_page(thread_current()->pagedir, file) != NULL)
	    f->eax = exec(file);
	  else
	    f->eax = -1;
	  break;
      case (SYS_WAIT):
          parse_syscall_args((uint32_t*) f->esp, 1, &pid);
          f->eax = wait(pid);
	  break;
      case (SYS_CREATE):
	  parse_syscall_args ((uint32_t*) f->esp, 2, &file, &size);
	  if (file == NULL)
	      f->eax = 0;  //e.g. false
          else if (pagedir_get_page(thread_current()->pagedir, file) == NULL)
      	      f->eax = -1; //e.g. error
          else
              f->eax = create(file, size);
	  break;
      case (SYS_OPEN):
          parse_syscall_args ((uint32_t*) f->esp, 1, &file);
          if (pagedir_get_page(thread_current()->pagedir, file) != NULL)
              f->eax = open(file);
          else
	      f->eax = -1;
          break;
      case (SYS_FILESIZE):
          parse_syscall_args ((uint32_t*) f->esp, 1, &fd);
          f->eax = filesize(fd);
	  break;
      case (SYS_READ):
	  parse_syscall_args ((uint32_t*) f->esp, 3, &fd, &buffer, &size);
	  if (buffer >= PHYS_BASE || buffer <= 0x08048000)
	      f->eax = -1;
	  else if (pagedir_get_page(thread_current()->pagedir, buffer) == NULL)
	      f->eax = -1;
	  else
	      f->eax = read (fd, buffer, size);
	  break;
      case (SYS_WRITE):
          parse_syscall_args ((uint32_t*) f->esp, 3, &fd, &buffer, &size);
	  if (pagedir_get_page(thread_current()->pagedir, buffer) != NULL)
	      f->eax = write(fd, buffer, size);
	  else
	      f->eax = -1;
	  break;
      case (SYS_SEEK):
          parse_syscall_args ((uint32_t*) f->esp, 2, &fd, &position);
          seek(fd, position);
          break;
      case (SYS_TELL):
	  parse_syscall_args ((uint32_t*) f->esp, 1, &fd);
          tell (fd);
	  break;
      case (SYS_CLOSE):
          parse_syscall_args ((uint32_t*) f->esp, 1, &fd);
          close (fd);
	  break;
      default:
	  printf("The syscall: %d is not supported.\n", syscall);
	  break;
  }
}


/*
 *  wait
 *  ----
 *  Purpose - To wait on a program to execute
 */
int
wait (pid_t pid)
{
  return process_wait(pid);
}

/*
 *  exec
 *  ----
 *  Purpose - Execute a program
 */
pid_t
exec (const char *cmd_line)
{
  pid_t pid;

  if (cmd_line == NULL || get_user(cmd_line) == -1 ||
      cmd_line >= PHYS_BASE || cmd_line <= 0x08048000)
     return -1;

  if (pagedir_get_page(thread_current()->pagedir, cmd_line) == 0)
    return -1;

  /* load the process by creating a new thread */
  pid = process_execute (cmd_line);

  if ((pid == PID_ERROR) || !is_loaded (pid))
    {
      find_thread(0)->child_exit_status = -1;
      return -1;
    }

  return pid;
}

/*
 * exit
 * ----
 * Purpose - Executes the exit system call
 */
void
exit (int status)
{
  struct thread *cur = thread_current ();

  cur->parent->child_exit_status = status;

  printf ("%s: exit(%d)\n", thread_name (), status);

  /* release the parent in process_wait() */
  sema_up (&cur->c_sema);

  thread_exit ();
}

/*
 *   create
 *   ------
 *   Purpose - to create a file
 */
bool
create (const char *file, unsigned size)
{
  //VET THE POINTER
  if (file == NULL || get_user(file) == -1 ||
      file >= PHYS_BASE || file <= 0x08048000)
  {
     return false;
  }

  return filesys_create(file, size);
}

/* Reads a byte at user virtual address UADDR.
   UADDR must be below PHYS_BASE.
   Returns the byte value if successful, -1 if a segfault
   occurred.

   Found @: https://w3.cs.jmu.edu/kirkpams/450-f15/projects/accessing_user_memory.shtml
   */
static int
get_user (const uint8_t *uaddr)
{
  int result;
  asm ("movl $1f, %0; movzbl %1, %0; 1:"
       : "=&a" (result) : "m" (*uaddr));
  return result;
}

/*
 *  close
 *  -----
 *  Purpose - To close a file
 */
void
close (int fd)
{
  if (has_fd(fd))
  {
      struct thread *cur = thread_current();

      int i, thread_fd_index = -1;

      // ITERATE THROUGH THE THREAD'S LOCAL FD_TABLE TO FIND THE MATCH
      for (i = 0; i < cur->thread_cur_fd_index; i++)
      {
	  if (cur->thread_fd_table[i] == fd)
	  {
	     thread_fd_index = i;
	     break;
	  }
      }

     //GET THE FILE
     struct file *file = fd_table[cur->thread_fd_table[thread_fd_index]];

     //TERMINATE THE FD IN THE LOCAL FD TABLE
     cur->thread_fd_table[thread_fd_index] = 0;

     //RETURN file_tell()
     return file_close(file);
  }
}

/*
 *  tell
 *  ----
 *  Purpose - To tell where the next byte read or written.
 */
int
tell (int fd)
{
  if (has_fd(fd))
  {
      struct thread *cur = thread_current();

      int i, thread_fd_index = -1;

      // ITERATE THROUGH THE THREAD'S LOCAL FD_TABLE TO FIND THE MATCH
      for (i = 0; i < cur->thread_cur_fd_index; i++)
      {
	  if (cur->thread_fd_table[i] == fd)
	  {
	     thread_fd_index = i;
	     break;
	  }
      }

     //GET THE FILE
     struct file *file = fd_table[cur->thread_fd_table[thread_fd_index]];

     //RETURN file_tell()
     return file_tell(file);
  }
  else
  {
      //printf("ERROR: This program doesn't have access to fd: %d\n", fd);
      return -1;
  }
}

/*
 *  halt
 *  ----
 *  Purpose - To shutdown the PintOS
 */
void
halt (void)
{
   shutdown_power_off();
}

/*
 *  has_file
 *  --------
 *  Purpose - To determine if a file exists within the cur thread's
 *            local fd_table
 */
bool
has_fd (int fd)
{
  //CHECK IF THE FILE DESCRIPTOR EXISTS IN THE CURRENT THREAD'S FD_TABLE
  struct thread *cur = thread_current();
  int i;

  // ITERATE THROUGH THE THREAD'S LOCAL FD_TABLE TO SEE IF WE CAN FIND A MATCH
  for (i = 0; i < cur->thread_cur_fd_index; i++)
  {
      if (cur->thread_fd_table[i] == fd)
      {
	 //A MATCH HAS BEEN FOUND
	 return true;
      }
  }

  return false;
}

/*
 *  filesize
 *  --------
 *  Purpose - To return the size of a file
 */
int
filesize (int fd)
{
  if (has_fd(fd))
  {
      struct thread *cur = thread_current();

      int i, thread_fd_index = -1;

      // ITERATE THROUGH THE THREAD'S LOCAL FD_TABLE TO SEE IF WE CAN FIND A MATCH
      for (i = 0; i < cur->thread_cur_fd_index; i++)
      {
	  if (cur->thread_fd_table[i] == fd)
	  {
	     thread_fd_index = i;
	     break;
	  }
      }

     //GET THE FILE
     struct file *file = fd_table[cur->thread_fd_table[thread_fd_index]];

     //RETURN THE FILE'S LENGTH
     return file_length(file);
  }
  else
  {
    //THIS THREAD DOESN'T HAVE ACCESS TO THIS FD
    return -1;
  }
}

/*
 *  seek
 *  ----
 *  Purpose - Execute the seek system call.
 */
void
seek (int fd, int position)
{
  //CHECK IF THE FILE DESCRIPTOR EXISTS IN THE CURRENT THREAD'S FD_TABLE
  struct thread *cur = thread_current();

  bool has_fd = false;        // DOES OUR THREAD HAVE THIS FD?
  int thread_fd_index = -1;   // IF SO... WHERE?
  int i;                      // ITERATOR

  // ITERATE THROUGH THE THREAD'S LOCAL FD_TABLE TO SEE IF WE CAN FIND A MATCH
  for (i = 0; i < cur->thread_cur_fd_index; i++)
  {
      if (cur->thread_fd_table[i] == fd)
      {
	 //A MATCH HAS BEEN FOUND
	 has_fd = true;
	 thread_fd_index = i;
	 break;
      }
  }

  if (has_fd)
  {
     //GET THE FILE
     struct file *file = fd_table[cur->thread_fd_table[thread_fd_index]];

     //MOVE TO FILE TO THE POSITION
     file_seek(file, position);
  }
}

/*
 * open
 * ----
 * Purpose - Executes the open system call
 */
int
open (const char *file_name)
{
  struct thread* cur = thread_current ();

  if (file_name == NULL || get_user(file_name) == -1 ||
      file_name >= PHYS_BASE || file_name <= 0x08048000)
  {
     return -1;
  }

  

  int index;
  struct file *file;

  file = filesys_open(file_name);

  if (file == NULL)
  {
    return -1;
  }

  //PUT THE FILE IN THE FD_TABLE
  fd_table[cur_fd_table_index] = file;

  //SET THE THREAD'S TABLE TO HOLD THE FD_TABLE INDEX
  cur->thread_fd_table[cur->thread_cur_fd_index] = cur_fd_table_index;

  //SET THE INDEX RETURN VAL
  index = cur_fd_table_index;

  //INCREMENT THE INDEX FOR THE FD_TABLE
  cur_fd_table_index++;

  //INCREMENT THE LOCAL THREADS INDEX
  cur->thread_cur_fd_index++;

  /*off_t offset = file_length(file);

  printf("file->length: %d\n", offset);

  printf("TEST: cur_fd_table_index: %d\n", cur_fd_table_index);*/

  return index;
}

/*
 * read
 * ----
 * Purpose - Executes the read system call
 */
int
read (int fd, void *buffer, unsigned size)
{

  //ERROR CHECKING FOR A BAD BUFFER POINTER
  if (buffer == NULL || get_user(buffer) == -1 ||
     (buffer + size) >= PHYS_BASE || buffer <= 0x08048000)
  {
     //ERROR: BAD BUFFER POINTER
     return -1;
  }

  unsigned i;

  if (fd == 0)
  {
      char *char_buffer = (char *) buffer;

     //READ FROM STDIN
     for (i = 0; i < size; i++) char_buffer[i] = input_getc();

     return size;
  }
  else if (fd == 1 || fd == 2)
  {
     //ERROR: TRYING TO READ FROM STDOUT OR STDERR
      return -1;
  }
  else //WE'LL TRY TO READ FROM A FILE
  {
      //CHECK IF THE FILE DESCRIPTOR EXISTS IN THE CURRENT THREAD'S FD_TABLE
      struct thread *cur = thread_current();

      bool has_fd = false;        // DOES OUR THREAD HAVE THIS FD?
      int thread_fd_index = -1;   // IF SO... WHERE?

      // ITERATE THROUGH THE THREAD'S LOCAL FD_TABLE TO SEE IF WE CAN FIND A MATCH
      for (i = 0; (int)i < cur->thread_cur_fd_index; i++)
      {
	  if (cur->thread_fd_table[i] == fd)
	  {
	     //A MATCH HAS BEEN FOUND
	     has_fd = true;
	     thread_fd_index = i;
	     break;
	  }
      }

      if (has_fd && (cur->thread_fd_table[thread_fd_index] <= cur_fd_table_index))
      {
	 // BEGIN READING FROM THE FILE
	 struct file *file = fd_table[cur->thread_fd_table[thread_fd_index]];

	 // TEMPORARLY HOLD THE RET VAL OF file_read()
	 int ret_val = file_read(file, buffer, size);

	 // SPIT-OUT WHAT WAS IN THE FILE FOR DEBUGGING PURPOSES ********************
	 //printf("buffer read: %s", char_buffer);

	 // NOW SEND BACK THE TEMP RET_VAL
	 return ret_val;
      }
      else
      {
	 // ERROR: THE FD WAS NOT FOUND IN THIS THREADS LOCAL FD_TABLE
	 return -1;
      }
  }
}


/*
 * write
 * ----
 * Purpose - Executes the write system call
 */
int
write (int fd, void *buffer, unsigned size)
{
  // BEGIN BY VETTING THE BUFFER
  //ERROR CHECKING FOR A BAD BUFFER POINTER
  if (buffer == NULL || get_user(buffer) == -1 ||
     (buffer + size) >= PHYS_BASE || buffer <= 0x08048000)
  {
     exit(-1);
     //ERROR: BAD BUFFER POINTER
     return -1;
  }

  int i;
  char* char_buffer = (char *)buffer;

  if (fd == 0 || fd == 2)
  {
    //ERROR: TRYING TO WRITE TO STDIN OR STDERR
    return -1;
  }
  else if (fd == 1)
  {
      //WRITE TO STDOUT
      putbuf (char_buffer, size);

      return size;
  }
  else //TRYING TO WRITE TO A FILE
  {
      //CHECK IF THE FILE DESCRIPTOR EXISTS IN THE CURRENT THREAD'S FD_TABLE
      struct thread *cur = thread_current();

      bool has_fd = false;        // DOES OUR THREAD HAVE THIS FD?
      int thread_fd_index = -1;   // IF SO... WHERE?

      // ITERATE THROUGH THE THREAD'S LOCAL FD_TABLE TO SEE IF WE CAN FIND A MATCH
      for (i = 0; (int)i < cur->thread_cur_fd_index; i++)
      {
	  if (cur->thread_fd_table[i] == fd)
	  {
	     //A MATCH HAS BEEN FOUND
	     has_fd = true;
	     thread_fd_index = i;
	     break;
	  }
      }

      if (has_fd && (cur->thread_fd_table[thread_fd_index] <= cur_fd_table_index))
      {
	 // BEGIN WRITING TO THE FILE
	 struct file *file = fd_table[cur->thread_fd_table[thread_fd_index]];

	 //USED FOR TESTING PURPOSES
	 //printf("writing to file: %s\n", char_buffer); //***********************************

	 // TEMPORARLY HOLD THE RET VAL OF file_read()
	 int ret_val = file_write(file, char_buffer, size);

	 // NOW SEND BACK THE TEMP RET_VAL
	 return ret_val;
      }
      else
      {
	 // THE FD WAS NOT FOUND
	  return -1;
      }

  }
}














