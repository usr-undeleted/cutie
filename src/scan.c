#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include "cutie-common.h"

void helpMenu() {
    printf("scan command basic usage:\n"
        "   scan <flags> <dirs>\n"
        "scan will search trough all directories you specificy.\n"
        "flags:\n"
        "   -h or --help: show this menu.\n\n"
        "scan is part of the cutie project hosted under https://github.com/usr-undeleted/cutie licensed under the GPLv3 license."
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

void printDir(DIR *dirStream, struct dirent *currentFile, char *currentDir) {
    size_t dirFileCap = 64;
    size_t dirFileCount = 0;
    size_t largestWordSize = 0;

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
        printf("%s:\n", currentDir);
    }

    // print
    for (int i = 0; i < dirFileCount; i++) {

        // ignore dotfiles
        if (entries[i].name[0] == '.') {
            continue;
        }

        if (entries[i].type != DT_DIR) { // if its a dir
            printf("%-*s\033[0m", (int)largestWordSize + 1, entries[i].name);
        } else { // other file type
            printf("\033[1m\033[34m%-*s\033[0m", (int)largestWordSize + 1, entries[i].name);
        }
    }

    // pretty it up
    printf("\n");

    // never forget!
    free(entries);
}

// call funcs to do work, handle errors
int main (int argc, char *argv[]) {
    char dir[PATH_MAX];
    DIR *dirStream;
    struct dirent *currentFile;

    if (argc == 1) { // get cwd
        dirStream = opendir(".");
        if (getcwd(dir, sizeof(dir)) != NULL) {
               // copied absolute cwd to dir
        } else {
               perror("Couldn't get absolute of current dir");
               return 1;
        }

    } else { // get dir user wants
        dirStream = opendir(argv[1]);
        strcpy(dir, argv[1]);
    }

    if (argc < 3) {
        singleDir = 1;
    }

    if (dirStream == NULL) {
        perror("Directory couldn't be opened");
        return 2;
    }

    printDir(dirStream, currentFile, dir);

    if (closedir(dirStream) == -1) {
        perror("Couldn't close directory");
        return 3;
    }

    return 0;
}
