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

const char PATH[15] = "/etc/myls.conf";

void clearFilterList(filterList *ftList)
{
    for (int i = 0; i < ftList->size; i++)
        free(ftList->arr[i]);
    ftList->size = 0;
    return;
}

void stripLine(char *line, char *dest)
{
    int lineLen = strlen(line);
    int i;
    for (i = 0; i < lineLen; i++)
    {
        char c = line[i];
        if (c == '#')
            break;
        dest[i] = c;
    }
}

int getWord(char *line, char *dest)
{
    int lineLen = strlen(line);
    int i = 0;
    while (i < lineLen)
    {
        if (isspace(line[i]))
            i++;
        else
            break;
    }
    int j = 0;
    while (i < lineLen)
    {
        if (isspace(line[i]))
        {
            break;
        }
        else
        {
            dest[j++] = line[i++];
        }
    }
    dest[j] = '\0';
    // printf("Got word : %s\n", dest);
    return i;
}

int permStrToMode(char *perm, mode_t *mode)
{
    if (perm[0] == 'r')
        *mode |= 0400;
    else if (perm[0] != '-')
        return -1;
    if (perm[1] == 'w')
        *mode |= 0200;
    else if (perm[1] != '-')
        return -1;
    if (perm[2] == 'x')
        *mode |= 0100;
    else if (perm[2] != '-')
        return -1;
    if (perm[3] == 'r')
        *mode |= 0040;
    else if (perm[3] != '-')
        return -1;
    if (perm[4] == 'w')
        *mode |= 0020;
    else if (perm[4] != '-')
        return -1;
    if (perm[5] == 'x')
        *mode |= 0010;
    else if (perm[5] != '-')
        return -1;
    if (perm[6] == 'r')
        *mode |= 0004;
    else if (perm[6] != '-')
        return -1;
    if (perm[7] == 'w')
        *mode |= 0002;
    else if (perm[7] != '-')
        return -1;
    if (perm[8] == 'x')
        *mode |= 0001;
    else if (perm[8] != '-')
        return -1;
    return 0;
}

int parseOption(char *option, filter *ft)
{
    // printf("parseOption - option: %s\n", option);
    int optionLen = strlen(option);
    // Check setuid
    if (strncmp(option, "setuid", optionLen + 1) == 0)
    {
        ft->flag_setuid = 1;
        return 0;
    }
    // Check setgid
    if (strncmp(option, "setgid", optionLen + 1) == 0)
    {
        ft->flag_setgid = 1;
        return 0;
    }
    // Check dot file
    if (strncmp(option, ".file", optionLen + 1) == 0)
    {
        ft->flag_dot = 1;
        return 0;
    }
    // Check worldwritable file
    if (strncmp(option, "worldwritable", optionLen + 1) == 0)
    {
        ft->flag_worldwritable = 1;
        return 0;
    }
    // Check mtime file
    if (strncmp(option, "MMIN:", 5) == 0)
    {
        char *tmp = malloc(optionLen);
        for (int i = 5; i < optionLen; i++)
        {
            tmp[i - 5] = option[i];
        }
        ft->mmin_n = atoi(tmp);
        ft->flag_mmin = 1;
        free(tmp);
        return 0;
    }
    // Check atime file
    if (strncmp(option, "AMIN:", 5) == 0)
    {
        char *tmp = malloc(optionLen);
        for (int i = 5; i < optionLen; i++)
        {
            tmp[i - 5] = option[i];
        }
        ft->amin_n = atoi(tmp);
        ft->flag_amin = 1;
        free(tmp);
        return 0;
    }
    // Check ctime file
    if (strncmp(option, "CMIN:", 5) == 0)
    {
        char *tmp = malloc(optionLen);
        for (int i = 5; i < optionLen; i++)
        {
            tmp[i - 5] = option[i];
        }
        ft->cmin_n = atoi(tmp);
        ft->flag_cmin = 1;
        free(tmp);
        return 0;
    }

    return -1;
}

int parseOptions(char *options, filter *ft)
{
    int optionsLen = strlen(options);
    int i = 0;
    char *option = malloc(optionsLen);
    memset(option, 0, optionsLen);
    int j = 0;
    while (i < optionsLen)
    {
        char c = options[i];
        if (c == ',')
        {
            if (parseOption(option, ft) < 0)
            {
                free(option);
                perror("Fail to parse a option");
                printf("option: %s\n", options);
                return -1;
            }
            memset(option, 0, optionsLen);
            j = 0;
        }
        else
        {
            option[j++] = c;
        }
        i++;
    }
    if (parseOption(option, ft) < 0)
    {
        free(option);
        perror("Fail to parse a option");
        printf("option: %s\n", options);
        return -1;
    }
    free(option);
    return 0;
}

int parseLine(char *line, filter *ft)
{
    // Remove comment
    int lineLen = strlen(line);
    char *stripedLine = malloc(lineLen);
    memset(stripedLine, 0, lineLen);
    stripLine(line, stripedLine);
    // Get path
    lineLen = strlen(stripedLine);
    char *path = malloc(lineLen);
    memset(path, 0, lineLen);
    int i = getWord(stripedLine, path);
    if (strlen(path) < 1)
    {
        perror("Fail to get a path from the line");
        printf("stripedLine: %s\n", stripedLine);
        free(path), free(stripedLine);
        return -1;
    }
    char *abPath = malloc(PATH_MAX);
    memset(abPath, 0, sizeof(*abPath) * PATH_MAX);
    realpath(path, abPath);
    strcpy(ft->abPath, abPath);
    free(path), free(abPath);
    // Get options
    lineLen -= i;
    char *options = malloc(lineLen);
    memset(options, 0, lineLen);
    getWord(stripedLine + i, options);
    free(stripedLine);
    if (strlen(options) < 1)
    {
        perror("Fail to get options from the line");
        printf("stripedLine: %s\n", stripedLine);
        free(options);
        return -1;
    }
    // Parse options
    if (parseOptions(options, ft) < 0)
    {
        free(options);
        perror("Fail to parse options");
        printf("options: %s\n", options);
        return -1;
    }
    return 0;
}

int parseConf(char *path, filterList *ftList)
{
    // Set conf file path
    char confPath[PATH_MAX];
    if (path == NULL)
        strcpy(confPath, PATH);
    else
        strcpy(confPath, path);

    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    fp = fopen(confPath, "r");
    if (fp == NULL)
    {
        printf("confPath: %s\n", confPath);
        perror("Fail to open conf file");
        return -1;
    }
    while ((nread = getline(&line, &len, fp)) != -1)
    {
        filter *ft = malloc(sizeof(filter));
        memset(ft, 0, sizeof(*ft));
        if (parseLine(line, ft) < 0)
        {
            perror("Fail to parse a line");
            free(ft);
            return -1;
        }
        else
        {
            ftList->arr[(ftList->size)++] = ft;
        }
    }

    fclose(fp);
    if (line)
        free(line);
    return 0;
}
