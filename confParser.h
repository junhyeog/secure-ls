#define FT_MAX 1024

typedef struct _filter
{
    char abPath[PATH_MAX]; // Absolute path

    bool flag_setuid;
    bool flag_setgid;
    bool flag_dot;
    bool flag_worldwritable;
    bool flag_amin;
    bool flag_cmin;
    bool flag_mmin;

    int amin_n;
    int cmin_n;
    int mmin_n;
} filter;

typedef struct _filterList
{
    filter *arr[FT_MAX];
    int size;
} filterList;

void clearFilterList(filterList *ftList);
void stripLine(char *line, char *dest);
int getWord(char *line, char *dest);
int parseOption(char *option, filter *ft);
int parseOptions(char *options, filter *ft);
int parseLine(char *line, filter *ft);
int parseConf(char *path, filterList *ftList);
