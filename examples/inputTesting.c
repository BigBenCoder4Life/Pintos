#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <syscall.h>


static void read_line (char line[], size_t);
static bool backspace (char **pos, char line[]);

int
main(int argc, char *argv[])
{
   char command[80];

   printf("Enter a string: ");
   read_line(command, sizeof command);
  

   
   printf("The string entered was: %s\n", command );

   return 0;
}


static void
read_line (char line[], size_t size)
{
  char *pos = line;
  for (;;)
    { 
      char c;
      read (STDIN_FILENO, &c, 1);

      switch (c)
        {
        case '\r':
          *pos = '\0';
          putchar ('\n');
          return;

        case '\b':
          backspace (&pos, line);
          break;

        case ('U' - 'A') + 1:       /* Ctrl+U. */
          while (backspace (&pos, line))
            continue;
          break;

        default: 
          /* Add character to line. */
          if (pos < line + size - 1)
            { 
              putchar (c);
              *pos++ = c;
            }
          break;
        }
    }
}


static bool
backspace (char **pos, char line[])
{
  if (*pos > line)
    {
      /* Back up cursor, overwrite character, back up
         again. */
      printf ("\b \b");
      (*pos)--;
      return true;
    }
  else
    return false;
}

