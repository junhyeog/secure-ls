typedef struct _filter
{
    char path[PATH_MAX];

    bool flag_setuid;
    bool flag_setgid;
    bool flag_dot;
    bool flag_worldwritable;
    bool flag_perm;
    bool flag_type;
    bool flag_uname;
    bool flag_gname;
    bool flag_amin;
    bool flag_cmin;
    bool flag_mmin;

    char perm[9];
    char type;
    char uname[NAME_MAX];
    char gname[NAME_MAX];
    int amin_n;
    int cmin_n;
    int mmin_n;
} filter;

void stripLine(char *line, char *dest);
int getWord(char *line, char *dest);
int permStrToMode(char *perm, mode_t *mode);
int parseOption(char *option, filter *ft);
int parseOptions(char *options, filter *ft);
int parseLine(char *line, filter *ft);
int parseConf(char *path, filter *ft);
