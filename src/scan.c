#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "cutie-common.h"

void helpMenu() {
    printf("scan command basic usage:\n"
        "   scan <flags> <dirs>\n"
        "scan will search trough all directories you specify.\n"
        "flags:\n"
        "   -h or --help: show this menu.\n"
        "   -a or --all: show all files, as, by default, scan hides dotfiles.\n\n"
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
    return strcmp(((struct entry *)a)->name, ((struct entry *)b)->name);
}

// decide when to print dir name; if only one dir searched, dont
int singleDir = 0;
// show dotfiles or not
int dotFiles = 0;

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
    if (!singleDir) {
        printf("\033[34m%s\033[0m:\n", currentDir);
    }

    // print
    for (int i = 0; i < dirFileCount; i++) {

        // ignore dotfiles
        if (entries[i].name[0] == '.' && !dotFiles) {
            continue;
        }

        char *colorCode = determineColor(entries[i].name);

        if (entries[i].type == DT_DIR) { // if its a dir
            char displayName[256];
            snprintf(displayName, sizeof(displayName), "%s/", entries[i].name);
            printf("\033[1m\033[34m%s\033[0m\n", displayName);
        } else { // other file type
            printf("\033[%sm%-*s\033[0m\n", colorCode,(int)largestWordSize - (int)strlen(entries[i].name), entries[i].name);
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
        'a'
    };
    char *stringFlags[] = {
        "--help",
        "--all"
    };
    int charLen = sizeof(charFlags) / sizeof(charFlags[0]);
    int stringLen = sizeof(stringFlags) / sizeof(stringFlags[0]);

    size_t flagCount;
    int *flags = labelFlags(argc, argv, charFlags, charLen, stringFlags, stringLen, &flagCount);

    if (flags != NULL) {
        for (int i = 0; i < flagCount; i++) {
            if (flags[i] == 0) helpMenu();

            if (flags[i] == 1) dotFiles = 1;
        }
    } else {
        printf("Invalid flag detected. See 'scan -h' or 'scan --help' for instructions.\n");
        return 1;
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
               return 2;
        }

        printDir(dirStream, dir);
        closedir(dirStream);

    } else { // get dir user wants
        for (int i = argc - 1; i > 0; i--) { // print dirs user wants on reverse order
            if (argv[i][0] != '-') {
                dirStream = opendir(argv[i]);
                strcpy(dir, argv[i]);

                if (dirStream == NULL && errno == ENOTDIR) {
                    char resolved[PATH_MAX];
                    if (realpath(argv[i], resolved) != NULL) {
                        printf("%s\n\n", resolved);
                    }

                    continue;

                } else if (dirStream == NULL) {
                    printf("Directory %s couldn't be opened: %s\n\n", argv[i], strerror(errno));
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
        printf("\033[A");
    }

    return 0;
}
