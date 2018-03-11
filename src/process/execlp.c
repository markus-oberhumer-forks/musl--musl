#include <unistd.h>
#include <errno.h>
#include <stdarg.h>

extern int __execsh(const char *, char *const []);

int execlp(const char *file, const char *argv0, ...)
{
	int argc;
	va_list ap;
	va_start(ap, argv0);
	for (argc=1; va_arg(ap, const char *); argc++);
	va_end(ap);
	{
		int i;
		char *argv[argc+1];
		va_start(ap, argv0);
		argv[0] = (char *)argv0;
		for (i=1; i<argc; i++)
			argv[i] = va_arg(ap, char *);
		argv[i] = NULL;
		va_end(ap);
		execvp(file, argv);
		if (errno == ENOEXEC) {
			errno = 0;
			return __execsh(file, argv);
		}
		return -1;
	}
}
