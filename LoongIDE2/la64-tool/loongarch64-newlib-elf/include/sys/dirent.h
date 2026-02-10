/* <dirent.h> includes <sys/dirent.h>, which is this file.  On a
   system which supports <dirent.h>, this file is overridden by
   dirent.h in the libc/sys/.../sys directory.  On a system which does
   not support <dirent.h>, we will get this file which uses #error to force
   an error.  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NAME_MAX
#define NAME_MAX 	255
#endif

typedef struct _dirdesc
{
	int	  dd_fd;
	long  dd_loc;
	long  dd_size;
	char *dd_buf;
	int	  dd_len;
	long  dd_seek;
	void *priv;
} DIR;

#define __dirfd(dp)		((dp)->dd_fd)

DIR *opendir(const char *);
struct dirent *readdir(DIR *);
int closedir(DIR *);
int rmdir(const char *);

#include <sys/types.h>

#include <limits.h>

struct dirent
{
	long  d_ino;
	off_t d_off;
	unsigned int d_attrs;
	unsigned short d_reclen;
	/* we need better syntax for variable-sized arrays */
	unsigned short d_namlen;
	char d_name[NAME_MAX + 1];
};

int scandir(const char *dirname,
			struct dirent *** namelist,
			int (*select)(const struct dirent *),
			int (*dcomp)(const struct dirent **, const struct dirent **)
);

#ifdef __cplusplus
}
#endif
