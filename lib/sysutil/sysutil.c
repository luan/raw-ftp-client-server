#include "sysutil.h"

char *strtrim(char **ptr) {
    char *initial = *ptr;

    while ((**ptr) == ' ')
        (*ptr)++;

    int i = strlen(*ptr) - 1;

    while ((*ptr)[i] == ' ')
        i--;

    (*ptr)[++i] = '\0';
    return initial;
}

int file_exists(const char *filename) {
    struct stat buffer;
    if (stat(filename, &buffer))
        return 0;

    int is_dir = ((int)(buffer.st_mode / 4096) == 4);
    if (is_dir)
        return 0;

    return 1;
}

unsigned long file_size(const char *filename) {
    struct stat buffer;
    if (stat(filename, &buffer))
        return 0;
    
    return buffer.st_size;
}

char* get_permissions(char *octa) {
    char *ret = (char*) calloc(11, sizeof(char));

    if (octa[0] == '7')
        strcat(ret, "rwx");
    else if (octa[0] == '6')
        strcat(ret, "rw-");
    else if (octa[0] == '5')
        strcat(ret, "r-x");
    else if (octa[0] == '4')
        strcat(ret, "r--");
    else if (octa[0] == '3')
        strcat(ret, "-wx");
    else if (octa[0] == '2')
        strcat(ret, "-w-");
    else if (octa[0] == '1')
        strcat(ret, "--r");
    else if (octa[0] == '0')
        strcat(ret, "---");

    if (octa[1] == '7')
        strcat(ret, "rwx");
    else if (octa[1] == '6')
        strcat(ret, "rw-");
    else if (octa[1] == '5')
        strcat(ret, "r-x");
    else if (octa[1] == '4')
        strcat(ret, "r--");
    else if (octa[1] == '3')
        strcat(ret, "-wx");
    else if (octa[1] == '2')
        strcat(ret, "-w-");
    else if (octa[1] == '1')
        strcat(ret, "--r");
    else if (octa[1] == '0')
        strcat(ret, "---");

    if (octa[2] == '7')
        strcat(ret, "rwx");
    else if (octa[2] == '6')
        strcat(ret, "rw-");
    else if (octa[2] == '5')
        strcat(ret, "r-x");
    else if (octa[2] == '4')
        strcat(ret, "r--");
    else if (octa[2] == '3')
        strcat(ret, "-wx");
    else if (octa[2] == '2')
        strcat(ret, "-w-");
    else if (octa[2] == '1')
        strcat(ret, "--r");
    else if (octa[2] == '0')
        strcat(ret, "---");

    return ret;
}

t_opt get_options(const char *cmd) {
    int i = 0;
    t_opt opt;

    opt.a = 0;
    opt.l = 0;

    while (cmd[i] != '-' && cmd[i] != '\0')
        i++;

    if (i) {
        opt.path = (char *) calloc(i + 1, sizeof(char));
        strncpy(opt.path, cmd, i++);
    }
    else {
        opt.path = (char *) calloc(i + 1, sizeof(char));
        strcpy(opt.path, ".");
    }

    strtrim(&opt.path);
    
    while (cmd[i] != '\0') {
        if (cmd[i] == 'a')
            opt.a = 1;
        if (cmd[i] == 'l')
            opt.l = 1;
        i++;
    }

    return opt;
}

int ls(const char *params, char *dest) {
    t_opt opt = get_options(params);
    DIR *dir = opendir(opt.path);
    char *current_path = get_current_dir_name();
    chdir(opt.path);

    struct dirent *ent;
    struct stat buf;
    register struct passwd *pw;
    register struct group *gw;

    if (dir == NULL)
        return 0;

    char *acc = (char *) malloc(sizeof(char) * 6);
    char last_modify[32];
    char *perm;
    int is_dir;
    int max_user = 0;
    int max_group = 0;
    int fsize_len = 1;
    unsigned long max_filesize = 0;

    if (opt.l) {
        while ((ent = readdir(dir)) != NULL) {
            if (!((ent->d_name[0] == '.') && !opt.a)) {
                stat(ent->d_name, &buf);

                pw = getpwuid(buf.st_uid);
                gw = getgrgid(buf.st_gid);

                if (buf.st_size > max_filesize)
                    max_filesize = buf.st_size;

                if (pw && strlen(pw->pw_name) > max_user)
                    max_user = strlen(pw->pw_name);

                if (gw && strlen(gw->gr_name) > max_group)
                    max_group = strlen(gw->gr_name);
            }
        }

        while (max_filesize / 10) {
            max_filesize /= 10;
            fsize_len++;
        }

        dir = opendir(opt.path);
    }

    while ((ent = readdir(dir)) != NULL) {
        if (opt.l) {
            if (!((ent->d_name[0] == '.') && !opt.a)) {
                stat(ent->d_name, &buf);
                sprintf(acc, "%o", buf.st_mode % 4096);

                perm = get_permissions(acc);

                is_dir = ((int)(buf.st_mode / 4096) == 4);

                pw = getpwuid(buf.st_uid);
                gw = getgrgid(buf.st_gid);

                char tmp[18];
                strftime(tmp, 18, "%Y-%m-%d %H:%M", gmtime(&(buf.st_mtime)));
                strcpy(last_modify, tmp);

                char format[64];
                sprintf(format, "%%9s %%-%d%c %%-%d%c %%%dld %%17s %%-25s\n", max_user,
                        (pw) ? 's' : 'd', max_group,
                        (gw) ? 's' : 'd', fsize_len);

                if (is_dir)
                    sprintf(dest + strlen(dest), "d");
                else
                    sprintf(dest + strlen(dest), "-");

                sprintf(dest + strlen(dest), format, perm, 
                        (pw) ? (char *) pw->pw_name : (char *) buf.st_uid, 
                        (gw) ? (char *) gw->gr_name : (char *) buf.st_gid,
                        buf.st_size, last_modify, ent->d_name);
            }
        }
        else if (!opt.l) {
            if (!((ent->d_name[0] == '.') && !opt.a)) {
                sprintf(dest + strlen(dest), "%s\n", ent->d_name);
            }
        }
    }

    closedir(dir);
    chdir(current_path);
    if (current_path != NULL)
        free(current_path);
    return 1;
    /*
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
       return retval;*/
}

int cd(const char *params) {
    return chdir(params);
}
