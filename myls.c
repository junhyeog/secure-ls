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
        if (~(mode & (S_IREAD >> i * 3)))
            f = 0;
        if (~(mode & (S_IWRITE >> i * 3)))
            f = 0;
    }
    return f;
}

int filtering(char *filepath, char *filename, struct stat *st, filter *ft)
{
    // Check setuid
    if (ft->flag_setuid && (st->st_mode & S_ISUID) != 0)
    {
        printWarn("setuid");
    }
    // Check setgid
    if (ft->flag_setgid && (st->st_mode & S_ISGID) != 0)
    {
        printWarn("setgid");
    }
    // Check dot file
    if (strlen(filename) >= 1 && filename[0] == '.')
    {
        printWarn("dot");
    }
    // Check worldwritable file
    if (chkWorldwritable(st->st_mode))
    {
        printWarn("world-writable");
    }
    // Check PERM
    return 0;
}

int main(int argc, char *argv[])
{
    filter ft;
    char test[12] = "./test.conf";
    printf("%d\n", parseConf(test, &ft));
    printf("%s\n", ft.path);
    printf("%d\n", ft.flag_setuid);
    printf("%d\n", ft.flag_setgid);
    printf("%d\n", ft.flag_worldwritable);
    printf("%d\n", ft.flag_dot);
    printf("%d\n", ft.flag_setgid);
}
