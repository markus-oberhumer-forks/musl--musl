#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "syscall.h"

static inline int test_symlink_dirness(int fd, const char *pathname)
{
	struct stat statbuf;
	if (fstatat(fd, pathname, &statbuf, AT_SYMLINK_NOFOLLOW) == -1) {
		return 1;
	}
	if (S_ISLNK(statbuf.st_mode)) {
		if (fstatat(fd, pathname, &statbuf, 0) == -1) return 1;

		if (S_ISDIR(statbuf.st_mode)) return 0;
		else {
			errno = ENOTDIR;
			return -1;
		}
	}
	return 1;
}

int renameat(int oldfd, const char *old, int newfd, const char *new)
{
	char old_copy[PATH_MAX+1], new_copy[PATH_MAX+1];
	char *base;
	size_t old_size, new_size;
	struct stat oldstat, newstat;
	int r;

	if ((old_size = strlen(old)) > PATH_MAX || \
	    (new_size = strlen(new)) > PATH_MAX) {
		errno = ENAMETOOLONG;
		return -1;
	}

	if (old_size == 0 || new_size == 0) {
		errno = ENOENT;
		return -1;
	}

	/* Test equality of old and new.
	   If they both resolve to the same dentry, we do nothing. */
	if (fstatat(oldfd, old, &oldstat, AT_SYMLINK_NOFOLLOW) == 0 && \
	    fstatat(newfd, new, &newstat, AT_SYMLINK_NOFOLLOW) == 0 && \
	    oldstat.st_dev == newstat.st_dev && \
	    oldstat.st_ino == newstat.st_ino) return 0;

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

	/* The Linux kernel will fail when attempting to rename a symlink of a
	   directory with a trailing slash.  We therefore have to handle this
	   case ourselves. */
	if (old[old_size - 1] == '/') {
		/* Calling stat(2) on a symlink to a dir with the trailing
		   slash causes stat(2) to return the actual directory instead
		   of the symlink itself. */
		strcpy(old_copy, old);
		old_copy[old_size - 1] = '\0';
		r = test_symlink_dirness(oldfd, old_copy);
		if (r == 0) old = old_copy;
		else if (r == -1) return -1;
	}

	if (new[new_size - 1] == '/') {
		strcpy(new_copy, new);
		new_copy[new_size - 1] = '\0';
		r = test_symlink_dirness(newfd, new_copy);
		if (r == 0) new = new_copy;
		else if (r == -1) return -1;
	}

	return syscall(SYS_renameat, oldfd, old, newfd, new);
}
