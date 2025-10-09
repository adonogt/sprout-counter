/* minimal newlib syscalls for cortex-m3, no os, no semihosting
 * comments are lowercase
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>

/* weak helper */
#ifndef WEAK
#define WEAK __attribute__((weak))
#endif

/* environment */
char *__env[1] = { 0 };
char **environ = __env;

/* pid/kill/exit */
WEAK int _getpid(void) { return 1; }

WEAK int _kill(int pid, int sig)
{
  (void)pid; (void)sig;
  errno = EINVAL;
  return -1;
}

WEAK void _exit(int status)
{
  (void)status;
  for (;;) { __asm volatile ("wfi"); }
}

/* i/o stubs (no uart by default) */
WEAK ssize_t _read(int file, void *ptr, size_t len)
{
  (void)file; (void)ptr; (void)len;
  errno = ENOSYS;
  return -1;
}

WEAK ssize_t _write(int file, const void *ptr, size_t len)
{
  (void)file; (void)ptr;
  /* swallow bytes to keep printf non-blocking */
  return (ssize_t)len;
}

/* fs stubs */
WEAK int _close(int file)                   { (void)file; errno = ENOSYS; return -1; }
WEAK int _isatty(int file)                  { (void)file; return 1; }
WEAK int _fstat(int file, struct stat *st)  { (void)file; if (!st){errno=EINVAL;return -1;} st->st_mode = S_IFCHR; return 0; }
WEAK off_t _lseek(int file, off_t off, int whence)
{
  (void)file; (void)off; (void)whence;
  errno = ENOSYS;
  return (off_t)-1;
}

/* memory: no heap for now */
WEAK void *_sbrk(ptrdiff_t incr)
{
  (void)incr;
  errno = ENOMEM;
  return (void *)-1;
}
