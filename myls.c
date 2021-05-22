#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <stddef.h>
#include <time.h>
#include <stdbool.h>
// #include <sys/mkdev.h>
#include <sys/sysmacros.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>

#include "confParser.h"

void printWarn(char *option)
{
    printf("[!] Warning: %s file found.\n", option);
    return;
}

bool chkWorldwritable(mode_t mode)
{
    int i;
    bool f = 1;
    for (i = 0; i < 3; i++)
    {
        if (!(mode & (S_IREAD >> i * 3)))
            f = 0;
        if (!(mode & (S_IWRITE >> i * 3)))
            f = 0;
    }
    return f;
}

int filtering(char *abPath, char *filename, struct stat *st, filterList *ftList)
{
    int flag = 0;
    for (int i = 0; i < ftList->size; i++)
    {
        filter *ft = (ftList->arr)[i];
        if (strncmp(abPath, ft->abPath, strlen(abPath) + 1) != 0)
            continue;
        // Check setuid
        if (ft->flag_setuid && (st->st_mode & S_ISUID) != 0)
        {
            printWarn("setuid");
            flag = 1;
        }
        // Check setgid
        if (ft->flag_setgid && (st->st_mode & S_ISGID) != 0)
        {
            printWarn("setgid");
            flag = 1;
        }
        // Check dot file
        if (strlen(filename) >= 1 && filename[0] == '.')
        {
            if (!(strlen(filename) == 1))                                        // not .
                if (!(strlen(filename) == 2 && strncmp(filename, "..", 3) == 0)) // not ..
                {
                    printWarn("dot");
                    flag = 1;
                }
        }
        // Check worldwritable file
        if (chkWorldwritable(st->st_mode))
        {
            printWarn("world-writable");
            flag = 1;
        }
        // Check time
        time_t cur = time(NULL);
        int dif;
        dif = (int)difftime(cur, st->st_mtime);
        if (ft->flag_mmin && dif / 60 <= ft->mmin_n)
        {
            printWarn("mtime");
            flag = 1;
        }

        dif = (int)difftime(cur, st->st_atime);
        if (ft->flag_amin && dif / 60 <= ft->amin_n)
        {
            printWarn("atime");
            flag = 1;
        }

        dif = (int)difftime(cur, st->st_ctime);
        if (ft->flag_cmin && dif / 60 <= ft->cmin_n)
        {
            printWarn("ctime");
            flag = 1;
        }
    }
    return flag;
}

/* typeOfFile - return the letter indicating the file type. */
char typeOfFile(mode_t mode)
{
    switch (mode & S_IFMT)
    {
    case S_IFREG:
        return ('-');
    case S_IFDIR:
        return ('d');
    case S_IFCHR:
        return ('c');
    case S_IFBLK:
        return ('b');
    case S_IFLNK:
        return ('l');
    case S_IFIFO:
        return ('p');
    case S_IFSOCK:
        return ('s');
    }
    return ('?');
}

/* permOfFile - return the file permissions in an "ls"-like string. */
char *permOfFile(mode_t mode)
{
    int i;
    char *p;
    static char perms[10];
    p = perms;
    strcpy(perms, "---------");
    for (i = 0; i < 3; i++)
    {
        if (mode & (S_IREAD >> i * 3))
            *p = 'r';
        p++;
        if (mode & (S_IWRITE >> i * 3))
            *p = 'w';
        p++;
        if (mode & (S_IEXEC >> i * 3))
            *p = 'x';
        p++;
    }
    if ((mode & S_ISUID) != 0)
        perms[2] = 's';
    if ((mode & S_ISGID) != 0)
        perms[5] = 's';
    return (perms);
}

/* outputStatInfo - print out the contents of the stat structure. */
void outputStatInfo(char *pathname, char *filename, struct stat *st)
{
    int n;
    char slink[BUFSIZ + 1];
    printf("%5d ", st->st_blocks);
    printf("%c%s ", typeOfFile(st->st_mode), permOfFile(st->st_mode));
    printf("%3d ", st->st_nlink);

    //  printf("%5d/%-5d ", st->st_uid, st->st_gid);
    struct passwd *pw = getpwuid(st->st_uid);
    struct group *gr = getgrgid(st->st_gid);
    printf("%.7s %.7s", pw->pw_name, gr->gr_name);

    if (((st->st_mode & S_IFMT) != S_IFCHR) && ((st->st_mode & S_IFMT) != S_IFBLK))
        printf("%9d ", st->st_size);
    else
        printf("%4d,%4d ", major(st->st_rdev), minor(st->st_rdev));

    printf("%.12s ", ctime(&st->st_mtime) + 4);
    printf("%s", filename);
    if ((st->st_mode & S_IFMT) == S_IFLNK)
    {
        if ((n = readlink(pathname, slink, sizeof(slink))) < 0)
            printf(" -> ???");
        else
            printf(" -> %.*s", n, slink);
    }
}

void usage()
{
    printf("./myls [-S] <path1> <path2> ...\n");
}

int main(int argc, char *argv[])
{

    if (argc <= 1)
    {
        usage();
        exit(EXIT_FAILURE);
    }

    bool flag_s = 0;
    int i = 1;
    if (strncmp("-S", argv[1], 3) == 0)
    {
        flag_s = 1;
        i++;
    }
    filterList *ftList = malloc(sizeof(filterList));
    memset(ftList, 0, sizeof(*ftList));
    char test[12] = "./test.conf";
    parseConf(test, ftList);

    DIR *dp;
    char *dirname;
    struct stat st;
    struct dirent *d;
    char filename[BUFSIZ + 1];
    /* For each directory on the command line... */
    for (; i < argc; i++)
    {

        dirname = realpath(argv[i], NULL);
        /* Open the directory */
        if ((dp = opendir(dirname)) == NULL)
        {
            perror(dirname);
            continue;
        }
        printf("%s:\n", argv[i]);

        while ((d = readdir(dp)) != NULL)
        { /* For each file in the directory... */
            /* Create the full file name. */
            sprintf(filename, "%s/%s", dirname, d->d_name);
            if (lstat(filename, &st) < 0) /* Find out about it. */
                perror(filename);
            if (flag_s)
            {

                if (filtering(dirname, d->d_name, &st, ftList))
                {
                    outputStatInfo(filename, d->d_name, &st); /* Print out the info. */
                    putchar('\n');
                }
            }
            else
            {
                outputStatInfo(filename, d->d_name, &st); /* Print out the info. */
                putchar('\n');
            }
        }
        putchar('\n');
    }
    clearFilterList(ftList);
    closedir(dp);
    free(dirname);
    return 0;
}
