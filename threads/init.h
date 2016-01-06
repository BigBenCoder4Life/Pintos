#ifndef THREADS_INIT_H
#define THREADS_INIT_H

#include <debug.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Page directory with kernel mappings only. */
extern uint32_t *init_page_dir;

/* This will be our file descriptor table */
extern struct file *fd_table[];
extern int cur_fd_table_index;

#endif /* threads/init.h */
