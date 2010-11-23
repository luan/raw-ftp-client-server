#ifndef _SYSUTIL_
    #define _SYSUTIL_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

void strtrim(char **ptr);
int ls(const char *params, char *dest);
int cd(const char *params);

#endif
