#ifndef _SYSUTIL_
    #define _SYSUTIL_
#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <ctype.h>

#include <errno.h>

typedef struct t_opt {
    int l;
    int a;
    int h;
    char *path;
} t_opt;


int file_exists(const char *filename);
unsigned long file_size(const char *filename);
unsigned long long free_disk_space();
char* get_permissions(char *octa);
t_opt get_options(const char *cmd);
char *strtrim(char **ptr);
int ls(const char *params, char *dest);
int cd(const char *params);

#endif
