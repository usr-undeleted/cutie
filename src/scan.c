#include <asm-generic/errno-base.h>
#include <dirent.h>
#include <linux/limits.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include "cutie-common.h"

// proper spacing for -l categories
unsigned int linksLargest = 0;
unsigned int ownerLargest = 0;
unsigned int groupLargest = 0;
unsigned int sizeLargest = 0;

// decide when to print dir name; if only one dir searched, dont
unsigned int singleDir = 0;
// show dotfiles or not
unsigned int dotFiles = 0;
// use a '/' after a dir or not
unsigned int useBar = 0;
// list or not
unsigned int fullList = 0;
// immediately open directories, if found
unsigned int beRecursive = 0;

void helpMenu(char *invocation) {
    printf("\e[1m%s\e[0m command basic usage:\n"
        "   \e[1m%s\e[0m <flags> <dirs>\n"
        "   \e[3;2mnote that the order of flags and dirs dont matter.\e[0m\n"
        "\e[1m%s\e[0m will search trough all directories you specify.\n\n"
        "flags:\n"
        "   \e[1m-h\e[0m or \e[1m--help\e[0m: show this menu.\n"
        "   \e[1m-a\e[0m or \e[1m--all\e[0m: show all files, as, by default, \e[1m%s\e[0m hides dotfiles.\n"
        "   \e[1m-c\e[0m or \e[1m--color\e[0m: toggle color.\n"
        "   \e[1m-b\e[0m or \e[1m--bar\e[0m: toggle the use of a '/' after directories.\n"
        "   \e[1m-l\e[0m or \e[1m--list\e[0m: show extra info on all files, showing permissions, links, owner, group, size and date of last edit.\n"
        "   \e[1m-r\e[0m or \e[1m--recursive\e[0m: whenever found, immediately open a folder and check its contents.\n\n"
        "\e[2;3m%s is part of the cutie project hosted under https://github.com/usr-undeleted/cutie licensed under the GPLv3 license.\e[0m\n",
        invocation, invocation, invocation, invocation, invocation
    );
    exit(0);
}

// qsort
int cmpEntries(const void *a, const void *b) {
    return strcasecmp(((struct entry *)a)->name, ((struct entry *)b)->name);
}

void printFile (struct entry entry, char *fullPath, int spacing, struct stat *st) {
    if (entry.name[0] == '.' && !dotFiles) {
        return;
    }

    // are we gonna wrap this?
    char apostrophe = strchr(entry.name, ' ') ? 39 : '\0';
    // toggles
    char bar = '\0';

    // info for -l
    char perms[11];
    size_t links;
    char *owner;
    char *group;
    size_t size;
    // formatted time
    time_t mtime;
    char ftime[64];
    // we already deal with the type

    if (fullList) {
        perms[0] = '?';
        if (S_ISREG (st->st_mode)) perms [0] = '-';
        if (S_ISDIR (st->st_mode)) perms [0] = 'd';
        if (S_ISLNK (st->st_mode)) perms [0] = 'l';
        if (S_ISCHR (st->st_mode)) perms [0] = 'c';
        if (S_ISBLK (st->st_mode)) perms [0] = 'b';
        if (S_ISFIFO(st->st_mode)) perms [0] = 'p';
        if (S_ISSOCK(st->st_mode)) perms [0] = 's';
        // usr perms
        perms[1] = st->st_mode & S_IRUSR ? 'r' : '-';
        perms[2] = st->st_mode & S_IWUSR ? 'w' : '-';
        perms[3] = st->st_mode & S_IXUSR ? 'x' : '-';
        // group
        perms[4] = st->st_mode & S_IRGRP ? 'r' : '-';
        perms[5] = st->st_mode & S_IWGRP ? 'w' : '-';
        perms[6] = st->st_mode & S_IXGRP ? 'x' : '-';
        // others
        perms[7] = st->st_mode & S_IROTH ? 'r' : '-';
        perms[8] = st->st_mode & S_IWOTH ? 'w' : '-';
        perms[9] = st->st_mode & S_IXOTH ? 'x' : '-';
        perms[10] = '\0';
        links = st->st_nlink;
        struct passwd *pw = getpwuid(st->st_uid);
        owner = pw ? pw->pw_name : "???";
        struct group *gr = getgrgid(st->st_gid);
        group = gr ? gr->gr_name : "???";
        size = st->st_size;
        // formatted time
        mtime = st->st_mtime;
        struct tm *time = localtime(&mtime);
        strftime(ftime, sizeof(ftime), "%b %d %H:%M", time);
    }

    char printed[5000];
    size_t colorLen;
    char *colorCode = determineColor(entry, st, &colorLen);
    if (!colorCode) colorCode = "0";

    // bar + apostrophes, color code
    if (apostrophe == '\0') {
        snprintf(printed, sizeof(printed), "\033[%.*sm%s%c", (int)colorLen, colorCode, entry.name, bar);
    } else {
        snprintf(printed, sizeof(printed), "\033[%.*sm%c%s%c%c", (int)colorLen, colorCode,
            apostrophe, entry.name, apostrophe, bar);
    }

    if (entry.type == DT_LNK) {
        char target[NAME_MAX + 10];
        ssize_t len = readlink(fullPath, target, sizeof(target) - 1);
        if (len > 0) {
            target[len] = '\0';
            strcat(printed, "\033[0m -> ");
            strcat(printed, target);
        }
    }

    if (fullList) {
        printf("%s %*zu %-*s %-*s %*zu %s %s\e[0m\n",
            perms, (int)linksLargest, links,
            (int)ownerLargest, owner,
            (int)groupLargest, group,
            (int)sizeLargest, size,
            ftime, printed);

    } else {
        if (!beRecursive) {
            // non recursive
            printf("%-*s\033[0m  ", spacing, printed);

        } else {
            // find where we should stop printing the full path
            // used to print everything but the file blue
            size_t nameStart;
            for (int i = 0; i < strlen(fullPath); i++) {
                if (fullPath[i] == '/') {
                    if (!strcmp(fullPath + (i + 1), entry.name)) {
                        nameStart = i + 1;
                        break;
                    }
                }
            }
            if (useColor) printf("\e[34;1m");
            printf("%.*s", (int)nameStart, fullPath);
            // then we print file
            printf("%s", printed);
            // do we need to print a bar?
            if (useBar && entry.type == DT_DIR) putchar('/');
            printf("\033[0m\n");
        }
    }
}

void printDir(DIR *dirStream, char *currentDir, struct winsize *dimensions) {
    size_t dirFileCap = 64;
    size_t dirFileCount = 0;
    size_t largestWordSize = 0;
    struct dirent *currentFile;

    struct entry *entries = malloc(dirFileCap * sizeof(*entries));
    if (entries == NULL) {
        printf("Failed to get files; failed malloc.\n");
        exit(2);
    }

    // populate entries
    while ((currentFile = readdir(dirStream)) != NULL) {

        if (strlen(currentFile->d_name) > largestWordSize) {
            largestWordSize = strlen(currentFile->d_name);
        }

        // increase the size of the dir's entries
        if (dirFileCount >= dirFileCap) {
            dirFileCap *= 2;
            entries = realloc(entries, dirFileCap * sizeof(*entries));
            if (entries == NULL) {
                printf("Failed to get files; failed malloc.\n");
                exit(2);
            }
        }

        strcpy(entries[dirFileCount].name, currentFile->d_name);
        entries[dirFileCount].type = currentFile->d_type;

        dirFileCount++;
    }

    // sort alphabeticallly
    qsort(entries, dirFileCount, sizeof(struct entry), cmpEntries);

    // say dir name
    char bar = useBar ? '/' : '\0';
    if (!singleDir && !fullList) {
        printf("%s%c:\n", currentDir, bar);
    }

    char resolved[PATH_MAX];

    if (fullList) {
        // we lstat once, then store the results here
        struct stat *stats = malloc(dirFileCount * sizeof(struct stat));
        // get the spacing for owner, groups, links, and size.
        for (int i = 0; i < dirFileCount; i++) {
            snprintf(resolved, sizeof(resolved), "%s/%s", currentDir, entries[i].name);

            if (lstat(resolved, &stats[i]) != 0) {
                printf("Failed to get stats on file: stat() failed.\n");
                if (!singleDir) {
                    continue;
                } else {
                    exit(2);
                }
            }

            size_t len;
            len = snprintf(NULL, 0, "%zu", stats[i].st_nlink);
            if (len > linksLargest) linksLargest = len;

            char *owner = getpwuid(stats[i].st_uid)->pw_name;
            if (strlen(owner) > ownerLargest) ownerLargest = strlen(owner);

            char *group = getgrgid(stats[i].st_gid)->gr_name;
            if (strlen(group) > groupLargest) groupLargest = strlen(group);

            len = snprintf(NULL, 0, "%zu", stats[i].st_size);
            if (len > sizeLargest) sizeLargest = len;
        }

        // call printFile
        for (int i = 0; i < dirFileCount; i++) {
            snprintf(resolved, sizeof(resolved), "%s/%s", currentDir, entries[i].name);
            printFile(entries[i], resolved, 0, &stats[i]);
        }
        // never forget!
        free(entries);
        free(stats);
        return;
    }

    int n = dirFileCount;
    // if we dont want dotfiles
    if (!dotFiles) {
        for (int i = 0; i < dirFileCount; i++) {
            if (entries[i].name[0] == '.') {
                n--;
            }
        }
    }
    size_t rows;
    // contains largest word length + spacing (2)
    size_t colWidth[n];
    // candidate for column size
    int c;
    // do we hide files?
    int visible[dirFileCount];
    int visibleN = 0;
    for (int i = 0; i < dirFileCount; i++) {
        if (entries[i].name[0] == '.' && !dotFiles) continue;
        visible[visibleN++] = i;
    }

    // get column size
    for (c = n; c >= 1; c--) {
        rows = (n + c - 1) / c;
        size_t total = 0;

        for (int i = 0; i < c; i++) {
            colWidth[i] = 0;

            for (int j = 0; j < rows; j++) {
                int idx = i * rows + j;
                if (idx < visibleN) {
                    int realIdx = visible[idx];
                    int len = strlen(entries[realIdx].name);

                    if (useBar && entries[realIdx].type == DT_DIR) len++;
                    if (len > colWidth[i]) colWidth[i] = len;
                }
            }
            total += colWidth[i];
        }
        total += (c - 1) * 2;

        // if we dont have enough space
        if (total <= dimensions->ws_col) {
            break;
        }
    }

    // lets print!
    for (int j = 0; j < rows; j++) {
        for (int i = 0; i < c; i++) {
            int idx = i * rows + j;
            int realIdx = 0;
            if (idx < visibleN) {
                realIdx = visible[idx];
                int len = strlen(entries[realIdx].name);

                if (useBar && entries[realIdx].type == DT_DIR) len++;
                if (len > colWidth[i]) colWidth[i] = len;
            }

            if (idx < visibleN) {
                snprintf(resolved, sizeof(resolved), "%s/%s", currentDir, entries[realIdx].name);
                struct stat st;

                if (lstat(resolved, &st) == 0) {
                    size_t colorLen;

                    char *colorCode = determineColor(entries[realIdx], &st, &colorLen);
                    if (!colorCode) colorCode = "0";

                    char bar = useBar && entries[realIdx].type == DT_DIR ? '/' : '\0';
                    char displayName[PATH_MAX + 2];
                    if (bar) {
                        snprintf(displayName, sizeof(displayName), "%s/", entries[realIdx].name);
                    } else {
                        snprintf(displayName, sizeof(displayName), "%s", entries[realIdx].name);
                    }
                    printf("\033[%.*sm%s\033[0m%-*s", (int)colorLen, colorCode, displayName,
                        (int)(colWidth[i] - strlen(displayName)), "");
                }
            } else if (i < c - 1) {
                printf("%-*s", (int)colWidth[i], "");

            }
            if (i < c - 1) {
                printf("  ");

            }
        }
        printf("\n");
    }

    // never forget!
    free(entries);
}

// take a directory, scan EVERYTHING In it
void scanRecursive (char *currentPath) {
    DIR *dirstream = opendir(currentPath);
    if (!dirstream) return;
    struct dirent *currentFile;

    while ((currentFile = readdir(dirstream)) != NULL) {
        // always hide '.' or '..'
        if (!strcmp(currentFile->d_name, ".") || !strcmp(currentFile->d_name, "..")) continue;
        if (currentFile->d_name[0] == '.' && !dotFiles) continue;

        char fullPath[PATH_MAX];
        if (currentPath[strlen(currentPath) - 1] == '/') {
            snprintf(fullPath, sizeof(fullPath), "%s%s", currentPath, currentFile->d_name);
        } else {
            snprintf(fullPath, sizeof(fullPath), "%s/%s", currentPath, currentFile->d_name);
        }

        struct stat st;
        lstat(fullPath, &st);

        struct entry fileEntry;
        strcpy(fileEntry.name, currentFile->d_name);
        fileEntry.type = currentFile->d_type;

        if (fileEntry.type != DT_DIR) {
            printFile(fileEntry, fullPath, 0, &st);
        } else {
            printFile(fileEntry, fullPath, 0, &st);
            scanRecursive(fullPath);
        }
    }
}

// call funcs to do work, handle errors
int main (int argc, char *argv[]) {
    char dir[PATH_MAX];
    DIR *dirStream;
    struct winsize dimensions;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &dimensions);
    // used by determineColor
    lsColors = getenv("LS_COLORS");

    // exclude flags from total arg count
    int totalFlags = 0;
    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == '-') {
            totalFlags++;
        }
    }

    // manage flags
    char charFlags[] = {
        'h',
        'a',
        'c',
        'b',
        'l',
        'r'
    };
    char *stringFlags[] = {
        "--help",
        "--all",
        "--color",
        "--bar",
        "--list",
        "--recursive"
    };
    int charLen = sizeof(charFlags) / sizeof(charFlags[0]);
    int stringLen = sizeof(stringFlags) / sizeof(stringFlags[0]);
    struct flagInput input = {
        charFlags, charLen,
        stringFlags, stringLen,
        0
    };

    int *flags = labelFlags(argc, argv, &input);

    if (flags != NULL) {
        for (int i = 0; i < input.flagCount; i++) {
            if (flags[i] == 0) helpMenu(argv[0]);

            if (flags[i] == 1) dotFiles = 1;

            if (flags[i] == 2) useColor = 1;

            if (flags[i] == 3) useBar = 1;

            if (flags[i] == 4) fullList = 1;

            if (flags[i] == 5) beRecursive = 1;
        }
    } else {
        printf("Invalid flag detected. See 'scan -h' or 'scan --help' for instructions.\n");
        return 2;
    }

    if (beRecursive) {
        if (argc == 1) {
            scanRecursive(".");
        } else {
            for (int i = 1; i < argc; i++) {
                scanRecursive(argv[i]);
            }
        }

        return 0;
    }

    free(flags);
    if (useColor) {
        // populate offsets
        size_t len = strlen(lsColors);
        parsedColorCount = 0;
        for (int i = 0; i < len; i++) {
            if (lsColors[i] == '*') {
                parsedColorCount++;
                void *tmp = realloc(parsedColors, parsedColorCount * sizeof(struct colorEntry));
                if (tmp == NULL) {
                    printf("Failed to get files; failed malloc.\n");
                    exit(2);
                }
                parsedColors = tmp;

                // starts at dot
                size_t tmpExtStart = i + 1;
                // ends at =, exclusive
                size_t tmpExtEnd = 0;
                // starts after =
                size_t tmpClrStart = 0;
                // is at :, exclusive
                size_t tmpClrEnd = 0;
                for (int j = i; lsColors[j] != ':'; j++) {
                    if (lsColors[j] == '=') {
                        tmpExtEnd = j;
                        tmpClrStart = j + 1;
                    }
                    if (lsColors[j + 1] == ':' || lsColors[j + 1] == '\0') {
                        tmpClrEnd = j + 1;
                    }
                }

                parsedColors[parsedColorCount - 1].extStart = tmpExtStart;
                parsedColors[parsedColorCount - 1].extEnd = tmpExtEnd;
                parsedColors[parsedColorCount - 1].colorStart = tmpClrStart;
                parsedColors[parsedColorCount - 1].colorEnd = tmpClrEnd;
            }
        }
    }

    if ((argc - totalFlags) < 3) {
        singleDir = 1;
    }

    if ((argc - totalFlags) <= 1) { // get cwd
        dirStream = opendir(".");

        if (getcwd(dir, sizeof(dir)) != NULL) {
               // copied absolute cwd to dir
        } else {
               perror("Couldn't get absolute of current dir");
               return 1;
        }

        printDir(dirStream, dir, &dimensions);
        closedir(dirStream);

    } else { // get dir user wants
        int onlyFail = 1;
        int hadDir = 0;
        int hadFile = 0;

        // first pass, print files
        for (int i = 1; i < argc; i++) {
            if (argv[i][0] != '-') {
                dirStream = opendir(argv[i]);
                strcpy(dir, argv[i]);

                if (dirStream == NULL && errno == ENOTDIR) {
                    onlyFail = 0;
                    hadFile = 1;
                    char resolved[PATH_MAX];
                    if ((realpath(argv[i], resolved)) == NULL) {
                        printf("Failed to get true path.\n");
                        exit(2);
                    }

                    struct stat st;
                    if ((stat(resolved, &st)) == -1) {
                        printf("Failed to get stats on file: stat() failed.\n");
                        if (!singleDir) {
                            continue;
                        } else {
                            exit(2);
                        }
                    }
                    unsigned char type = IFTODT(st.st_mode);

                    struct entry file;
                    strncpy(file.name, argv[i], NAME_MAX);
                    file.name[NAME_MAX] = '\0';
                    file.type = type;

                    if (singleDir) {
                        strncpy(file.name, resolved, NAME_MAX);
                        file.name[NAME_MAX] = '\0';
                    }
                    printFile(file, resolved, 1, &st);
                } else {
                    hadDir = 1;
                    continue;
                }
            }
        }

        if (hadDir && hadFile && !fullList) {
            printf("\n\n");
        } else if (!hadDir && hadFile) {
            printf("\n");
        }

        // second pass, print dirs and children
        if (hadDir) {
            for (int i = 1; i < argc; i++) {
                if (argv[i][0] != '-') {
                    dirStream = opendir(argv[i]);
                    strcpy(dir, argv[i]);

                    if (dirStream == NULL) {
                        if (errno == ENOTDIR) {
                            continue;
                        }
                        if (errno == ENOENT) {
                            printf("Directory '%s' coudln't be opened: %s\n\n", argv[i], strerror(errno));
                            if (!singleDir) {
                                continue;
                            } else {
                                printf("\033[A");
                                return 2;
                            }
                        }

                        printf("Directory or file '%s' couldn't be opened: %s\n\n", argv[i], strerror(errno));
                        if (singleDir) {
                            printf("\033[A");
                            return 2;
                        }
                        continue;
                    }

                    onlyFail = 0;

                    if (fullList && !singleDir) {
                        char bar = useBar ? '/' : '\0';
                        printf("\n%s%c:\n", argv[i], bar);
                        struct stat st;
                        lstat(argv[i], &st);

                        struct entry dirEntry;
                        strcpy(dirEntry.name, argv[i]);
                        dirEntry.type = DT_DIR;

                        //printFile(dirEntry, argv[i], 0, &st);

                    } else {
                        printDir(dirStream, dir, &dimensions);
                    }

                    printDir(dirStream, dir, &dimensions);

                    if (!singleDir && !beRecursive) {
                        //printf("\n");
                    }
                    closedir(dirStream);
                }
            }
        }

        if (!hadFile && hadDir && !singleDir) {
            //printf("\033[A");
        } else if (hadFile && hadDir) {
            //printf("\033[A");
        }

        if (onlyFail) {
            return 2;
        }

    }
    free(parsedColors);
    return 0;
}
