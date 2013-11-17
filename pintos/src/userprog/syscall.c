#include "userprog/syscall.h"
#include <console.h>
#include <debug.h>
#include <stdio.h>
#include <syscall-nr.h>
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/init.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/process.h"
#include "userprog/pagedir.h"
#include "devices/input.h"

static void syscall_handler (struct intr_frame *);
static void* get_arg (void** esp, int arg_no);
static void* check_user_vaddr (void* ptr);

/* Syscalls */
static void sys_halt (void);
static void sys_exit (int status);
static pid_t sys_exec (const char *cmd_line);
static int sys_wait (pid_t pid);
static bool sys_create (const char *file, unsigned initial_size);
static bool sys_remove (const char *file);
static int sys_open (const char *file);
static int sys_filesize (int fd);
int sys_read (int fd, void *buffer, unsigned size);
int sys_write (int fd, const void *buffer, unsigned size);
int sys_seek (int fd, unsigned position);
unsigned sys_tell (int fd);
void sys_close (int fd);

/* Filesys Functionalities */
static struct file* find_file_by_fd (struct thread*, int);
static int setup_fd (struct thread*, struct file*);
  
void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
  int* ptr = (int*)f->esp;
  int syscall_id = *ptr;

  switch (syscall_id)
  {
    case SYS_HALT:              /* Halt the operating system. */
      sys_halt();
      NOT_REACHED ();
      break;

    case SYS_EXIT:              /* Terminate this process. */
      sys_exit (*((int*)get_arg(&f->esp, 0)));
      NOT_REACHED ();
      break;

    case SYS_EXEC:              /* Start another process. */
      f->eax = sys_exec (*((char**)get_arg(&f->esp, 0)));
      break;

    case SYS_WAIT:              /* Wait for a child process to die. */
      f->eax = sys_wait (*((int*)get_arg(&f->esp, 0)));
      break;

    case SYS_CREATE:            /* Create a file. */
      f->eax = sys_create (*((char**)get_arg(&f->esp, 0)), \
                           *((unsigned*)get_arg(&f->esp, 1)));
      break;

    case SYS_REMOVE:            /* Delete a file. */
      f->eax = sys_remove (*((char**)get_arg(&f->esp, 0)));
      break;

    case SYS_OPEN:              /* Open a file. */
      f->eax = sys_open (*((char**)get_arg(&f->esp, 0)));
      break;

    case SYS_FILESIZE:          /* Obtain a file's size. */
      f->eax = sys_filesize (*((int*)get_arg(&f->esp, 0)));
      break;

    case SYS_READ:              /* Read from a file. */
      f->eax = sys_read (*((int*)get_arg(&f->esp, 0)), *((void**)get_arg(&f->esp, 1)), \
                         *((unsigned*)get_arg(&f->esp, 2)));
      break;

    case SYS_WRITE:             /* Write to a file. */
      f->eax = sys_write (*((int*)get_arg(&f->esp, 0)), *((void**)get_arg(&f->esp, 1)), \
                          *((unsigned*)get_arg(&f->esp, 2)));
      break;

    case SYS_SEEK:              /* Change position in a file. */
      sys_seek (*((int*)get_arg(&f->esp, 0)), *((unsigned*)get_arg(&f->esp, 1))); 
      break;

    case SYS_TELL:              /* Report current position in a file. */
      f->eax = sys_tell (*((int*)get_arg(&f->esp, 0)));
      break;

    case SYS_CLOSE:             /* Close a file. */
      sys_close (*((int*)get_arg(&f->esp,0)));
      break;

    default:
      printf("%d: UNKNOWN SYSCALL\n", syscall_id);
      break;
  }
}

static void*
get_arg (void** esp, int arg_no)
{
  return ((int*)*esp) + arg_no + 1;
}

static void*
check_user_vaddr (void *ptr)
{
  if ((ptr != NULL) && \
      (is_user_vaddr (ptr)) && \
      (pagedir_get_page (thread_current ()->pagedir, ptr) != NULL))
    return ptr;
  return NULL;
}

/* Syscalls */
void
sys_halt (void)
{
  power_off ();
}

void 
sys_exit (int status)
{
  struct thread *cur = thread_current ();
  cur->exit_status = status;
  thread_exit();
}

pid_t
sys_exec (const char *cmd_line)
{
  tid_t tid = process_execute (cmd_line);
  return tid;
}

int
sys_wait (pid_t pid)
{
  return process_wait (pid);
}

bool
sys_create (const char *file, unsigned initial_size)
{
  if (check_user_vaddr ((void*)file) != NULL)  
    return filesys_create (file, initial_size);

  return false;
}

bool
sys_remove (const char *file)
{
  if (check_user_vaddr ((void*)file) != NULL)
    return filesys_remove (file);
  return false;
}

int
sys_open (const char *file)
{
  struct thread *cur = thread_current ();
  struct file *f;
  
  if ((check_user_vaddr ((void*)file) != NULL) \
      && ((f = filesys_open (file)) != NULL))
  {
    return setup_fd (cur, f);
  }

  sys_exit(-1);
  NOT_REACHED ();
}

int
sys_filesize (int fd)
{
  struct thread *cur = thread_current ();
  struct file *f = find_file_by_fd (cur, fd);

  if (f != NULL)
    return file_length (f);
  else
    return -1;
}

int
sys_read (int fd, void *buffer, unsigned size)
{
  ASSERT (fd >= 0);
  unsigned i = 0;

  if (check_user_vaddr ((void*)buffer) != NULL)
  {
    if (fd == STDIN_FILENO)
    {
      for (i = 0; i < size; i++)
        ((uint8_t*)buffer)[i] = input_getc();
      
      return size;
    }
    else if (fd != STDOUT_FILENO)
    {
      struct thread *cur = thread_current ();
      struct file *f = find_file_by_fd (cur, fd);

      if (f != NULL)
        return file_read (f, buffer, size);
    }
  }

  sys_exit(-1);
  NOT_REACHED ();
}

int
sys_write (int fd, const void *buffer, unsigned size)
{
  ASSERT (fd >= 0);
  
  if (check_user_vaddr ((void*)buffer) != NULL)
  {
    if (fd == STDOUT_FILENO)
    {
      putbuf (buffer, size);
      return size;
    }
    else if (fd != STDIN_FILENO && fd > 2)
    {
      struct thread *cur = thread_current ();
      struct file *f = find_file_by_fd (cur, fd);

      if (f != NULL)
        return file_write (f, buffer, size);
    }
  }

  sys_exit (-1);
  NOT_REACHED ();
}

int
sys_seek (int fd, unsigned position)
{
  struct thread *cur = thread_current ();
  struct file *f = find_file_by_fd (cur, fd);

  if (f != NULL)
    file_seek (f, position);

  return -1;
}

unsigned
sys_tell (int fd)
{
  struct thread *cur = thread_current ();
  struct file *f = find_file_by_fd (cur, fd);

  if (f != NULL)
    return file_tell (f);

  return 0;
}

void
sys_close (int fd)
{
  struct thread *cur = thread_current ();
  struct file *f = find_file_by_fd (cur, fd);

  if (f != NULL)
  {
    list_remove (&f->elem);
    file_close (f);
  }
}

/* Find file by fd */
static struct file*
find_file_by_fd (struct thread *t, int fd)
{
  struct list_elem *e;
  struct file *f;
  struct list *file_list = &t->file_list;

  e = list_head (file_list);
  
  while ((e = list_next (e)) != list_end (file_list))
  {
    f = list_entry (e, struct file, elem);
    
    if (f->fd == fd)
      return f;
  }
  return NULL;
}

/* Generate new available file descriptor no. */
static int
setup_fd (struct thread *t, struct file *f)
{
  struct list_elem *e;
  struct file *f_;
  struct list *file_list = &t->file_list;
  int new_fd = 3;

  e = list_head (file_list);
  
  while ((e = list_next (e)) != list_end (file_list))
  {
    f_ = list_entry (e, struct file, elem);
    
    if (f_->fd != new_fd)
      break;
    else
      new_fd++;
  }

  f->fd = new_fd;
  list_insert (e, &f->elem);

  return new_fd;
}
