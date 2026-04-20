#include <linux/limits.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
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

void printDir(DIR *dirStream, char *currentDir) {
    size_t dirFileCap = 64;
    size_t dirFileCount = 0;
    size_t largestWordSize = 0;
    struct dirent *currentFile;

    struct entry *entries = malloc(dirFileCap * sizeof(*entries));

    // populate entries
    while ((currentFile = readdir(dirStream)) != NULL) {

        if ((int)strlen(currentFile->d_name) > (int)largestWordSize) {
            largestWordSize = (int)strlen(currentFile->d_name);
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
    char *bar = useBar ? "/" : "";
    if (!singleDir) {
        printf("%s%s:\n", currentDir, bar);
    }

    // print
    for (int i = 0; i < dirFileCount; i++) {

        // ignore dotfiles
        if (entries[i].name[0] == '.' && !dotFiles) {
            continue;
        }

        // full path to file
        char resolved[PATH_MAX];
        if ((realpath(entries[i].name, resolved)) == NULL && entries[i].type != DT_DIR) {
            printf("Couldn't resolve realpath for file.\n");
            exit(2);
        }
        char *colorCode = determineColor(resolved) ? determineColor(resolved) : "0";

        // we'll use this to check if executable
        struct stat st;
        // executable
        if (S_ISREG(st.st_mode) && (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))) {
                char *executableColor = useColor ? "32;1" : "0";
                printf("\033[%sm%s\033[0m\n", executableColor, entries[i].name);

        // directory
        } else if (entries[i].type == DT_DIR) {
            char displayName[256];
            char *color = useColor ? "34" : "0";
            char *bar = useBar ? "%s/" : "%s";

            snprintf(displayName, sizeof(displayName), bar, entries[i].name);
            printf("\033[1m\033[%sm%s\033[0m\n", color, displayName);

        // any other file
        } else {
            printf("\033[%sm%s\033[0m\n", colorCode, entries[i].name);
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
        // first pass, print files
        int hadDir = 0;
        int hadFile = 0;
        for (int i = 1; i < argc; i++) {
            if (argv[i][0] != '-') {
                dirStream = opendir(argv[i]);

                if (dirStream == NULL && errno == ENOTDIR) {
                    hadFile = 1;
                    char resolved[PATH_MAX];

                    struct stat st;

                    if (realpath(argv[i], resolved) != NULL) {
                        stat(resolved, &st);

                        // executable
                        if (S_ISREG(st.st_mode) && (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))) {
                            char *colorCode = useColor ? "32;1" : "0";

                            if (!singleDir) {
                                printf("\033[%sm%s\033[0m ", colorCode, argv[i]);
                            } else {
                                printf("\033[%sm%s\033[0m", colorCode, argv[i]);
                            }

                        // regular file
                        } else if (S_ISREG(st.st_mode)) {
                            char *colorCode = determineColor(resolved) ? determineColor(resolved) : "0";

                            if (!singleDir) {
                                printf("\033[%sm%s\033[0m ", colorCode, argv[i]);
                            } else {
                                printf("\033[%sm%s\033[0m", colorCode, resolved);
                            }
                        }

                    } else {
                        perror("File couldn't be opened");
                        return 1;
                    }

                    continue;
                } else {
                    hadDir = 1;
                }
            }
        }

        if (hadDir && hadFile) {
            printf("\n");
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

                        printf("Directory '%s' couldn't be opened: %s\n", argv[i], strerror(errno));
                        if (singleDir) {
                            return 1;
                        }
                        continue;
                    }

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
    }
    return 0;
}
