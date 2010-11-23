#include "sysutil.h"

void strtrim(char **ptr) {
    while ((**ptr) == ' ')
        (*ptr)++;

    int i = strlen(*ptr) - 1;

    while ((*ptr)[i] == ' ')
        i--;

    (*ptr)[++i] = '\0';
}

int ls(const char *params, char *dest) {
    char *cmd = (char *) malloc(sizeof(char) * (strlen(params) + 9));
    sprintf(cmd, "ls %s > .l", params);
    int retval = system(cmd);
    free(cmd);

    FILE *fp = fopen(".l", "r");
    char c;
    unsigned count = 0;

    while ((c = fgetc(fp)) != EOF) {
        count++;
        sprintf(dest, "%s%c", dest, c);
    }

    fclose(fp);
    system("rm -f .l");
    return retval;
}

int cd(const char *params) {
    return chdir(params);
}
