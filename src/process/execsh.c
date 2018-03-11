#include <unistd.h>
#include <errno.h>
#include "libc.h"

int
__execsh(const char *file, char *const argv[])
{
	int i, argc;
	char **p;

	for (argc=1, p=(char **)argv; *p; ++argc, ++p);

	char *nargv[argc+1];
	nargv[0] = (char *)file;
	for (i=0; i<argc; ++i)
		nargv[i+1] = argv[i];
	return execv("/bin/sh", nargv);
}
