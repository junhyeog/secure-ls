#include <sys/types.h>
// #include <sys/mkdev.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <grp.h>
#include <pwd.h>
#include <stdlib.h>

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

int main(int argc, char **argv)
{
    DIR *dp;
    char *dirname;
    struct stat st;
    struct dirent *d;
    char filename[BUFSIZ + 1];
    /* For each directory on the command line... */
    while (--argc)
    {
        dirname = realpath(*++argv, NULL);
        /* Open the directory */
        if ((dp = opendir(dirname)) == NULL)
        {
            perror(dirname);
            continue;
        }
        printf("%s:\n", *argv);

        while ((d = readdir(dp)) != NULL)
        { /* For each file in the directory... */
            /* Create the full file name. */
            sprintf(filename, "%s/%s", dirname, d->d_name);
            if (lstat(filename, &st) < 0) /* Find out about it. */
                perror(filename);
            outputStatInfo(filename, d->d_name, &st); /* Print out the info. */
            putchar('\n');
        }
        putchar('\n');
        closedir(dp);
        free(dirname);
    }
    return 0;
}
