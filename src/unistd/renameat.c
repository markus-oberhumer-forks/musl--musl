#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "syscall.h"

int renameat(int oldfd, const char *old, int newfd, const char *new)
{
	char old_copy[PATH_MAX+1], new_copy[PATH_MAX+1];
	char *base;

	if (strlen(old) > PATH_MAX || strlen(new) > PATH_MAX) {
		errno = ENAMETOOLONG;
		return -1;
	}

	if (strlen(old) == 0 || strlen(new) == 0) {
		errno = ENOENT;
		return -1;
	}

	strcpy(old_copy, old);
	strcpy(new_copy, new);

	base = basename(old_copy);
	strncpy(old_copy, base, sizeof(old_copy));
	base = basename(new_copy);
	strncpy(new_copy, base, sizeof(new_copy));

	if (strcmp(old_copy, ".") == 0 || strcmp(old_copy, "..") == 0 ||
	    strcmp(new_copy, ".") == 0 || strcmp(new_copy, "..") == 0) {
		errno = EINVAL;
		return -1;
	}

	return syscall(SYS_renameat, oldfd, old, newfd, new);
}
