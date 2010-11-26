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
        strcat(ret, "--x");
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
        strcat(ret, "--x");
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
        strcat(ret, "--x");
    else if (octa[2] == '0')
        strcat(ret, "---");

    return ret;
}

unsigned long long free_disk_space() {
    struct statvfs buffer;
    char *current_path = get_current_dir_name();
    statvfs(current_path, &buffer);
    free(current_path);

    return buffer.f_bavail * buffer.f_bsize;
}

t_opt get_options(const char *cmd) {
    int i = 0;
    t_opt opt;

    opt.a = 0;
    opt.l = 0;
    opt.h = 0;

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
        if (cmd[i] == 'h')
            opt.h = 1;
        i++;
    }

    return opt;
}

char *getfilesize_string(unsigned long filesize, int human) {
    char *ret;
    
    if (!human) {
        int len = 1;
        unsigned long size = filesize;
        while (size / 10) {
            size /= 10;
            len++;
        }
        ret = (char *) malloc(sizeof(char) * len);
        sprintf(ret, "%ld", filesize);

        return ret;
    }

    char units[] = "KMGT";
    double size = (double) filesize;
    int i = -1;
    ret = (char *) malloc(sizeof(char) * 6);

    while (size >= 1024) {
        size /= 1024.0;
        i++;
    }

    if (i >= 0)
        sprintf(ret, "%.1lf%c", size, units[i]);
    else
        sprintf(ret, "%ld", filesize);

    return ret;
}

int ls(const char *params, char *dest) {
    t_opt opt = get_options(params);
    DIR *dir = opendir(opt.path);
    char *current_path = get_current_dir_name();

    struct dirent *ent;
    struct stat buf;
    register struct passwd *pw;
    register struct group *gw;

    if (dir == NULL)
        return 0;

    chdir(opt.path);

    char *acc = (char *) malloc(sizeof(char) * 6);
    char last_modify[32];
    char *perm;
    int i;
    int is_dir;
    int is_exec;
    int max_user = 0;
    int max_group = 0;
    int fsize_len = 1;

    if (opt.l) {
        while ((ent = readdir(dir)) != NULL) {
            if (!((ent->d_name[0] == '.') && !opt.a)) {
                stat(ent->d_name, &buf);

                pw = getpwuid(buf.st_uid);
                gw = getgrgid(buf.st_gid);

                char *fsizestr = getfilesize_string(buf.st_size, opt.h);
                if (strlen(fsizestr) > fsize_len)
                    fsize_len = strlen(fsizestr);

                if (pw && strlen(pw->pw_name) > max_user)
                    max_user = strlen(pw->pw_name);

                if (gw && strlen(gw->gr_name) > max_group)
                    max_group = strlen(gw->gr_name);
            }
        }

        rewinddir(dir);
    }

    while ((ent = readdir(dir)) != NULL) {
        if (opt.l) {
            if (!((ent->d_name[0] == '.') && !opt.a)) {
                stat(ent->d_name, &buf);
                sprintf(acc, "%o", buf.st_mode % 4096);

                perm = get_permissions(acc);

                is_dir = ((int) (buf.st_mode / 4096) == 4);
                is_exec = 0;

               for (i = 2; i < 9; i += 3) {
                    if (perm[i] == 'x') {
                        is_exec = 1;
                        break;
                    }
                }

                pw = getpwuid(buf.st_uid);
                gw = getgrgid(buf.st_gid);

                char tmp[18];
                strftime(tmp, 18, "%Y-%m-%d %H:%M", gmtime(&(buf.st_mtime)));
                strcpy(last_modify, tmp);

                char format[64];
                char *fsizestr = getfilesize_string(buf.st_size, opt.h);
                sprintf(format, "%%9s %%-%d%c %%-%d%c %%%ds %%17s ", max_user,
                        (pw) ? 's' : 'd', max_group,
                        (gw) ? 's' : 'd', fsize_len);

                if (is_dir)
                    sprintf(dest + strlen(dest), "d");
                else
                    sprintf(dest + strlen(dest), "-");

                sprintf(dest + strlen(dest), format, perm, 
                        (pw) ? (char *) pw->pw_name : (char *) buf.st_uid, 
                        (gw) ? (char *) gw->gr_name : (char *) buf.st_gid,
                        fsizestr, last_modify);

                if (is_dir)
                    sprintf(dest + strlen(dest), "\033[1;34m");
                else if (is_exec)
                    sprintf(dest + strlen(dest), "\033[1;32m");

                sprintf(dest + strlen(dest), "%s", ent->d_name);

                if (is_dir || is_exec)
                    sprintf(dest + strlen(dest), "\033[0m");
                sprintf(dest + strlen(dest), "\n");
            }
        }
        else if (!opt.l) {
            if (!((ent->d_name[0] == '.') && !opt.a)) {
                stat(ent->d_name, &buf);
                is_dir = ((int) (buf.st_mode / 4096) == 4);
                is_exec = 0;
                sprintf(acc, "%o", buf.st_mode % 4096);
                perm = get_permissions(acc);

                for (i = 2; i < 9; i += 3) {
                    if (perm[i] == 'x') {
                        is_exec = 1;
                        break;
                    }
                }

                if (is_dir)
                    sprintf(dest + strlen(dest), "\033[1;34m");
                else if (is_exec)
                    sprintf(dest + strlen(dest), "\033[1;32m");

                sprintf(dest + strlen(dest), "%s", ent->d_name);

                if (is_dir || is_exec)
                    sprintf(dest + strlen(dest), "\033[0m");

                sprintf(dest + strlen(dest), "\n");
            }
        }
    }

    closedir(dir);
    chdir(current_path);
    free(current_path);
    return 1;
}

int cd(const char *params) {
    return chdir(params);
}
