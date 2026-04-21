#include <linux/limits.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include "cutie-common.h"

// decide when to print dir name; if only one dir searched, dont
int singleDir = 0;
// show dotfiles or not
int dotFiles = 0;
// use a '/' after a dir or not
int useBar = 0;
// color var located in cutie-common.h

void helpMenu() {
    printf("scan command basic usage:\n"
        "   scan <flags> <dirs>\n"
        "scan will search trough all directories you specify.\n"
        "flags:\n"
        "   -h or --help: show this menu.\n"
        "   -a or --all: show all files, as, by default, scan hides dotfiles.\n"
        "   -c or --color: toggle color.\n"
        "   -b or --bar: toggle the use of a '/' after directories.\n\n"
        "scan is part of the cutie project hosted under https://github.com/usr-undeleted/cutie licensed under the GPLv3 license.\n"
    );
    exit(0);
}

// will contain a whole directory
struct entry {
    char *name;
    unsigned char type; // DT_DIR, DT_REG, etc
};

// qsort
int cmpEntries(const void *a, const void *b) {
    return strcasecmp(((struct entry *)a)->name, ((struct entry *)b)->name);
}

void printFile (struct entry entry, char *fullPath) {
    if (entry.name[0] == '.' && !dotFiles) {
        return;
    }

    // are we gonna wrap this?
    char apostrophe = strchr(entry.name, ' ') ? 39 : '\0';

    char *colorCode;
    char bar = '\0';

    // we'll use this to check if executable
    struct stat st;
    stat(fullPath, &st);

    if (entry.type == DT_DIR) { // directory
        colorCode = useColor ? "34;1" : "0";
        bar = useBar ? '/' : '\0';

    } else if (entry.type == DT_LNK) { // symlink
        colorCode = useColor ? "36;1" : "0";

    } else if (S_ISREG(st.st_mode)
        && (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
        && entry.type != DT_DIR) { // executable
            colorCode = useColor ? "32;1" : "0";

    } else if (entry.type == DT_UNKNOWN || entry.type != DT_DIR) { // any other file
        colorCode = determineColor(entry.name) ? determineColor(entry.name) : "0";

    } else { // fallback
        colorCode = "0";
    }

    printf("\033[%sm%c%s%c%c  \033[0m", colorCode, apostrophe, entry.name, bar, apostrophe);
}

void printDir(DIR *dirStream, char *currentDir) {
    size_t dirFileCap = 64;
    size_t dirFileCount = 0;
    size_t largestWordSize = 0;
    struct dirent *currentFile;

    struct entry *entries = malloc(dirFileCap * sizeof(*entries));

    // populate entries
    while ((currentFile = readdir(dirStream)) != NULL) {

        if (strlen(currentFile->d_name) > largestWordSize) {
            largestWordSize = strlen(currentFile->d_name);
        }

        // increase the size of the dir's entries
        if (dirFileCount >= dirFileCap) {
            dirFileCap *= 2;
            entries = realloc(entries, dirFileCap * sizeof(*entries));
        }

        entries[dirFileCount].name = currentFile->d_name;
        entries[dirFileCount].type = currentFile->d_type;

        dirFileCount++;
    }

    // sort alphabeticallly
    qsort(entries, dirFileCount, sizeof(struct entry), cmpEntries);

    // say dir name
    char bar = useBar ? '/' : '\0';
    if (!singleDir) {
        printf("%s%c:\n", currentDir, bar);
    }
    char resolved[PATH_MAX];

    // get term size
    struct winsize dimensions;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &dimensions);
    int cols = dimensions.ws_col / (largestWordSize + 2);
    if (cols == 0) {
        cols = 1;
    }
    int rows = (dirFileCount + cols - 1) / cols;

    // print
    for (int row = 0; row < rows; row++) {
        if (cols < 2) { // regular
            for (int i = 0; i < dirFileCount; i++) {
                char fullPath[PATH_MAX];
                snprintf(fullPath, sizeof(fullPath), "%s/%s", currentDir, entries[i].name);
                realpath(fullPath, resolved);
                printFile(entries[i], resolved);

            }
            printf("\n");
            break;

        } else { // columns
            for (int col = 0; col < cols; col++) {
                int i  = row + col * rows;
                if (i < dirFileCount) {
                    if (entries[i].name[0] == '.' && !dotFiles) {
                        continue;
                    } else {
                        char fullPath[PATH_MAX];
                        snprintf(fullPath, sizeof(fullPath), "%s/%s", currentDir, entries[i].name);
                        realpath(fullPath, resolved);
                        printFile(entries[i], resolved);
                    }
                }
            }
            printf("\n");
        }
    }
    // never forget!
    free(entries);
}

// call funcs to do work, handle errors
int main (int argc, char *argv[]) {
    char dir[PATH_MAX];
    DIR *dirStream;

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
        'b'
    };
    char *stringFlags[] = {
        "--help",
        "--all",
        "--color",
        "--bar"
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
            if (flags[i] == 0) helpMenu();

            if (flags[i] == 1) dotFiles = 1;

            if (flags[i] == 2) useColor = 1;

            if (flags[i] == 3) useBar = 1;
        }
    } else {
        printf("Invalid flag detected. See 'scan -h' or 'scan --help' for instructions.\n");
        return 1;
    }
    free(flags);

    if ((argc - totalFlags) < 3) {
        singleDir = 1;
    }

    if ((argc - totalFlags) <= 1) { // get cwd
        dirStream = opendir(".");

        if (getcwd(dir, sizeof(dir)) != NULL) {
               // copied absolute cwd to dir
        } else {
               perror("Couldn't get absolute of current dir");
               return 2;
        }

        printDir(dirStream, dir);
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
                    struct entry file = {
                        argv[i],
                        DT_REG
                    };

                    char resolved[PATH_MAX];
                    if ((realpath(argv[i], resolved)) == NULL) {
                        printf("Failed to get true path.\n");
                        exit(2);
                    }

                    if (singleDir) {
                        file.name = resolved;
                    }
                    printFile(file, resolved);
                } else {
                    hadDir = 1;
                    continue;
                }
            }
        }

        if (hadDir && hadFile) {
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

                        printf("Directory '%s' couldn't be opened: %s\n\n", argv[i], strerror(errno));
                        if (singleDir) {
                            printf("\033[A");
                            return 1;
                        }
                        continue;
                    }

                    onlyFail = 0;
                    printDir(dirStream, dir);
                    if (!singleDir) {
                        printf("\n");
                    }
                    closedir(dirStream);
                }
            }
        }

        if (!hadFile && hadDir && !singleDir) {
            printf("\033[A");
        } else if (hadFile && hadDir) {
            printf("\033[A");
        }

        if (onlyFail) {
            return 1;
        }

    }
    return 0;
}
