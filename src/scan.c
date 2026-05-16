#include <asm-generic/errno-base.h>
#include <dirent.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
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
// return code for errors, same logic (1 program, 2 user)
int returnCode = 0;

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

void printFile (const char *name, unsigned char type, char *fullPath, int spacing, struct stat *st) {
    if (name[0] == '.' && !dotFiles) {
        return;
    }

    // are we gonna wrap this?
    char apostrophe = strchr(name, ' ') ? 39 : '\0';
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
        perms[3] = (st->st_mode & S_ISUID) ?
            ((st->st_mode & S_IXUSR) ? 's' : 'S') :
            ((st->st_mode & S_IXUSR) ? 'x' : '-');
        // group
        perms[4] = st->st_mode & S_IRGRP ? 'r' : '-';
        perms[5] = st->st_mode & S_IWGRP ? 'w' : '-';
        perms[6] = (st->st_mode & S_ISGID) ?
            ((st->st_mode & S_IXGRP) ? 's' : 'S') :
            ((st->st_mode & S_IXGRP) ? 'x' : '-');
        // others
        perms[7] = st->st_mode & S_IROTH ? 'r' : '-';
        perms[8] = st->st_mode & S_IWOTH ? 'w' : '-';
        perms[9] = (st->st_mode & S_ISVTX) ?
            ((st->st_mode & S_IXOTH) ? 't' : 'T') :
            ((st->st_mode & S_IXOTH) ? 'x' : '-');
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
    char *colorCode = determineColor(name, type, st, &colorLen);
    if (!colorCode) colorCode = "0";

    // bar + apostrophes, color code
    if (apostrophe == '\0') {
        snprintf(printed, sizeof(printed), "\033[%.*sm%s%c", (int)colorLen, colorCode, name, bar);
    } else {
        snprintf(printed, sizeof(printed), "\033[%.*sm%c%s%c%c", (int)colorLen, colorCode,
            apostrophe, name, apostrophe, bar);
    }

    if (type == DT_LNK) {
        char target[NAME_MAX + 10];
        ssize_t len = readlink(fullPath, target, sizeof(target) - 1);
        if (len > 0) {
            target[len] = '\0';
            // get coloring for target of symlink
            struct stat st;
            size_t colorLen;
            char *colorCode;

            if (useColor) {
                stat(fullPath, &st);
                unsigned char targetType = S_ISDIR(st.st_mode) ? DT_DIR :
                               S_ISREG(st.st_mode) ? DT_REG :
                               S_ISLNK(st.st_mode) ? DT_LNK :
                               S_ISCHR(st.st_mode) ? DT_CHR :
                               S_ISBLK(st.st_mode) ? DT_BLK :
                               S_ISSOCK(st.st_mode) ? DT_SOCK :
                               S_ISFIFO(st.st_mode) ? DT_FIFO : DT_UNKNOWN;

                colorCode = determineColor(name, targetType, &st, &colorLen);

            } else {
                colorCode = "0";
                colorLen = 1;
            }

            /*
            strcat(printed, "\e[0m -> \e[");
            strcat(printed, colorCode);
            strcat(printed, "m");
            strcat(printed, target);
            */
            size_t len = strlen(printed);
            snprintf(printed + len, sizeof(printed) - len, "\e[0m -> \e[%sm%s\e[0m", colorCode, target);
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
                    if (!strcmp(fullPath + (i + 1), name)) {
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
            if (useBar && type == DT_DIR) putchar('/');
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
        perror("Failed to allocate memory");
        exit(1);
    }

    // populate entries
    while ((currentFile = readdir(dirStream)) != NULL) {

        size_t len = strlen(currentFile->d_name);
        if (len > largestWordSize) {
            largestWordSize = len;
        }

        // increase the size of the dir's entries
        if (dirFileCount >= dirFileCap) {
            dirFileCap *= 2;
            entries = realloc(entries, dirFileCap * sizeof(*entries));
            if (entries == NULL) {
                perror("Failed to allocate memory");
                exit(1);
            }
        }

        strcpy(entries[dirFileCount].name, currentFile->d_name);
        entries[dirFileCount].type = currentFile->d_type;

        dirFileCount++;
    }

    // sort alphabeticallly
    qsort(entries, dirFileCount, sizeof(struct entry), cmpEntries);

    char resolved[PATH_MAX];

    linksLargest = 0;
    ownerLargest = 0;
    groupLargest = 0;
    sizeLargest = 0;
    if (fullList) {
        // we lstat once, then store the results here
        struct stat *stats = malloc(dirFileCount * sizeof(struct stat));
        long int totalBlocks = 0;
        if (stats == NULL) {
            perror("Failed to allocate memory");
            exit(1);
        }

        // get the spacing for owner, groups, links, and size.
        for (int i = 0; i < dirFileCount; i++) {
            snprintf(resolved, sizeof(resolved), "%s/%s", currentDir, entries[i].name);

            if (lstat(resolved, &stats[i]) != 0) {
                fprintf(stderr, "Failed to get stats on file '%s': %s", resolved, strerror(errno));
                if (!singleDir) {
                    returnCode = 2;
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

            totalBlocks += stats[i].st_blocks;
        }

        // show total blocks per directory
        printf("total %ld\n", totalBlocks / 2);

        // call printFile
        for (int i = 0; i < dirFileCount; i++) {
            snprintf(resolved, sizeof(resolved), "%s/%s", currentDir, entries[i].name);
            printFile(entries[i].name, entries[i].type, resolved, 0, &stats[i]);
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

            int realIdx = 0;
            for (int j = 0; j < rows; j++) {
                int idx = i * rows + j;
                if (idx < visibleN) {
                    realIdx = visible[idx];
                    int len = strlen(entries[realIdx].name);

                    if (useBar && entries[realIdx].type == DT_DIR) len++;
                    if (len > colWidth[i]) colWidth[i] = len;
                }
            }
            total += colWidth[i];
            // if theres a space, add space for two apostrophes
            if ((strchr(entries[realIdx].name, ' ')) != NULL) total += 2;
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
            }

            if (idx < visibleN) {
                snprintf(resolved, sizeof(resolved), "%s/%s", currentDir, entries[realIdx].name);
                struct stat st;

                if (lstat(resolved, &st) == 0) {
                    size_t colorLen;
                    char *colorCode;

                    if (useColor) {
                        colorCode = determineColor(entries[realIdx].name, entries[realIdx].type, &st, &colorLen);
                    } else {
                        colorCode = "0";
                        colorLen = 1;
                    }

                    char bar = useBar && entries[realIdx].type == DT_DIR ? '/' : '\0';
                    char displayName[PATH_MAX + 3];
                    char hasSpace = strchr(entries[realIdx].name, ' ') ? 1 : 0;
                    snprintf(displayName, sizeof(displayName), "%s%s%s%c",
                        hasSpace ? "'" : "", entries[realIdx].name, hasSpace ? "'" : "", bar);

                    printf("\033[%.*sm%s\033[0m%-*s", (int)colorLen, colorCode, displayName,
                        (int)(colWidth[i] - strlen(displayName)) + (bar ? 1 : 0), "");
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

// used by the first pass to do column printing
// different since we directly need to handle
// lists, not simply just a directory
// ... funny how I/O is easier here-
struct firstPassEntry {
    char name[PATH_MAX + 1];
    unsigned char type;
    struct stat st;
};
void firstPassPrint(struct firstPassEntry *entries, int count, struct winsize *dimensions) {
    int nameLens[count];
    int maxNameLen = 0;
    for (int i = 0; i < count; i++) {
        nameLens[i] = strlen(entries[i].name);
        if (nameLens[i] > maxNameLen) maxNameLen = nameLens[i];
    }

    if (fullList) {
        // get the spacing for owner, groups, links, and size.
        for (int i = 0; i < count; i++) {
            size_t len;
            len = snprintf(NULL, 0, "%zu", entries[i].st.st_nlink);
            if (len > linksLargest) linksLargest = len;

            char *owner = getpwuid(entries[i].st.st_uid)->pw_name;
            if (strlen(owner) > ownerLargest) ownerLargest = strlen(owner);

            char *group = getgrgid(entries[i].st.st_gid)->gr_name;
            if (strlen(group) > groupLargest) groupLargest = strlen(group);

            len = snprintf(NULL, 0, "%zu", entries[i].st.st_size);
            if (len > sizeLargest) sizeLargest = len;
        }

        // call printFile
        for (int i = 0; i < count; i++) {
            printFile(entries[i].name, entries[i].type, entries[i].name, 0, &entries[i].st);
        }
        // never forget!
        free(entries);
        return;
    }

    int n = count;
    // if we dont want dotfiles
    if (!dotFiles) {
        for (int i = 0; i < count; i++) {
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
    int visible[count];
    int visibleN = 0;
    for (int i = 0; i < count; i++) {
        if (entries[i].name[0] == '.' && !dotFiles) continue;
        visible[visibleN++] = i;
    }

    // get column size
    // find the longest name and try the theoritical maximum first
    int startC = n < (dimensions->ws_col / (maxNameLen + 2))
                 ? n
                 : (dimensions->ws_col / (maxNameLen + 2));

    for (c = startC; c >= 1; c--) {
        rows = (n + c - 1) / c;
        size_t total = 0;

        for (int i = 0; i < c; i++) {
            colWidth[i] = 0;

            int realIdx = 0;
            for (int j = 0; j < rows; j++) {
                int idx = i * rows + j;
                if (idx < visibleN) {
                    realIdx = visible[idx];
                    int len = nameLens[realIdx];

                    if (useBar && entries[realIdx].type == DT_DIR) len++;
                    if (len > colWidth[i]) colWidth[i] = len;
                }
            }
            total += colWidth[i];
            // if theres a space, add space for two apostrophes
            if ((strchr(entries[realIdx].name, ' ')) != NULL) total += 2;
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
                int len = nameLens[realIdx];

                if (useBar && entries[realIdx].type == DT_DIR) len++;
            }

            if (idx < visibleN) {
                size_t colorLen;
                char *colorCode;

                if (useColor) {
                    colorCode = determineColor(entries[realIdx].name, entries[realIdx].type, &entries[realIdx].st, &colorLen);
                } else {
                    colorCode = "0";
                    colorLen = 1;
                }

                char bar = useBar && entries[realIdx].type == DT_DIR ? '/' : '\0';
                char displayName[PATH_MAX + 3];
                char hasSpace = strchr(entries[realIdx].name, ' ') ? 1 : 0;
                snprintf(displayName, sizeof(displayName), "%s%s%s%c",
                    hasSpace ? "'" : "", entries[realIdx].name, hasSpace ? "'" : "", bar);

                printf("\033[%.*sm%s\033[0m%-*s", (int)colorLen, colorCode, displayName,
                    (int)(colWidth[i] - strlen(displayName)) + (bar ? 1 : 0), "");

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

        // form full path
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
            printFile(fileEntry.name, fileEntry.type, fullPath, 0, &st);
        } else {
            printFile(fileEntry.name, fileEntry.type, fullPath, 0, &st);
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
    // lets us be lazy
    setvbuf(stdout, NULL, _IOFBF, BUFSIZ * 2);
    // used by determineColor
    if (useColor) {
        lsColors = getenv("LS_COLORS");
        if (!lsColors) {
            lsColors = getenv("SCAN_COLORS");
            if (!lsColors) {
                returnCode = 2;
            }
        }
    }

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
        fprintf(stderr, "Invalid flag detected. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
        return 2;
    }

    free(flags);

    if (beRecursive) {
        if (argc - totalFlags == 1) {
            scanRecursive(".");
        } else {
            for (int i = 1; i < argc; i++) {
                scanRecursive(argv[i]);
            }
        }

        return 0;
    }

    if (useColor) {
        // populate offsets
        size_t len = strlen(lsColors);
        parsedColorCount = 0;
        for (int i = 0; i < len; i++) {
            if (lsColors[i] == '*') {
                parsedColorCount++;
                void *tmp = realloc(parsedColors, parsedColorCount * sizeof(struct colorEntry));
                if (tmp == NULL) {
                    perror("Failed to allocate memory");
                    exit(1);
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

    // decide if we should print extra newlines
    unsigned int needSeparator = 0;
    if ((argc - totalFlags) <= 1) { // get cwd
        dirStream = opendir(".");

        if ((getcwd(dir, sizeof(dir))) == NULL) {
            perror("Couldn't get absolute of current dir");
            return 2;
        }

        printDir(dirStream, dir, &dimensions);
        closedir(dirStream);

    } else { // get files user wants
        int onlyFail = 1;
        int hadDir = 0;
        int hadFile = 0;

        // first pass, print files
        // store processed entries in array
        struct firstPassEntry *firstPass = malloc(argc * sizeof(*firstPass));
        int firstPassCount = 0;
        for (int i = 1; i < argc; i++) {
            if (argv[i][0] == '-') continue;
            struct stat st;
            if (lstat(argv[i], &st) != 0) {
                fprintf(stderr, "Couldn't access '%s': %s\n", argv[i], strerror(errno));
                returnCode = 2;
                continue;
            }

            // if its a directory
            if (S_ISDIR(st.st_mode)) {
                hadDir = 1;
                continue;
            }

            onlyFail = 0;
            // populate file entry
            strcpy(firstPass[firstPassCount].name, argv[i]);
            firstPass[firstPassCount].name[PATH_MAX] = '\0';
            firstPass[firstPassCount].type = IFTODT(st.st_mode);
            firstPass[firstPassCount].st = st;
            firstPassCount++;
        }
        if (firstPassCount > 0) {
            firstPassPrint(firstPass, firstPassCount, &dimensions);
            if (hadDir) printf("\n");
        } else {
            free(firstPass);
        }

        if (hadFile && hadDir) printf("\n");

        // second pass, print dirs and children
        if (hadDir) {
            for (int i = 1; i < argc; i++) {
                if (argv[i][0] != '-') {
                    dirStream = opendir(argv[i]);
                    strcpy(dir, argv[i]);

                    if (needSeparator) printf("\n");

                    if (dirStream == NULL) {
                        if (errno == ENOTDIR) {
                            continue;
                        }
                        if (errno == ENOENT) {
                            fprintf(stderr, "Couldn't access '%s': %s\n", argv[i], strerror(errno));
                            needSeparator = 0;
                            if (!singleDir) {
                                returnCode = 2;
                                continue;
                            } else {
                                return 2;
                            }
                        }

                        fprintf(stderr, "Couldn't access '%s': %s\n", argv[i], strerror(errno));
                        needSeparator = 0;
                        if (singleDir) {
                            return 2;
                        } else {
                            returnCode = 2;
                        }
                        continue;
                    }

                    if (hadFile) printf("\n");

                    char bar = useBar ? '/' : '\0';
                    if (!singleDir) {
                        if (useColor) {
                            printf("\033[34;1m%s%c:\033[0m\n", argv[i], bar);
                        } else {
                            printf("%s%c:\n", argv[i], bar);
                        }
                    }

                    onlyFail = 0;

                    printDir(dirStream, dir, &dimensions);
                    if (!singleDir) printf("\n");
                    closedir(dirStream);
                }
            }
        }

        if (onlyFail) {
            return 2;
        }

    }
    free(parsedColors);
    return returnCode;
}
