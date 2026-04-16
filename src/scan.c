#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

void printDir(DIR *dirStream, struct dirent *currentFile) {
    size_t dirFileCap = 64;
    size_t dirFileCount = 0;
    size_t largestWordSize = 0;

    struct entry {
        char *name;
        unsigned char type; // DT_DIR, DT_REG, etc
    };

    struct entry *entries = malloc(dirFileCap * sizeof(*entries));

    while ((currentFile = readdir(dirStream)) != NULL) {

        if ((int)strlen(currentFile->d_name) > (int)largestWordSize) {
            largestWordSize = (int)strlen(currentFile->d_name);
        }

        // increase the size of the dir's entries
        if (dirFileCount >= dirFileCap) {
            dirFileCap += 64;
            entries = realloc(entries, dirFileCap * sizeof(*entries));
        }

        printf("%-*s\033[0m", (int)largestWordSize + 2, entries->name);

        //if (dirFileCount % 6 == 0) {
        //    printf("\n");
        //}

        entries->name = currentFile->d_name;
        entries->type = currentFile->d_type;

        dirFileCount++;
    }
}

// call funcs to do work, handle errors
int main (int argc, char *argv[]) {
    char *dir;
    DIR *dirStream;
    struct dirent *currentFile;

    if (argc == 1) { // get cwd
        dirStream = opendir(".");
    } else { // get dir user wants
        dirStream = opendir(argv[1]);
    }

    if (dirStream == NULL) {
        perror("Directory couldn't be opened");
        return 1;
    }

    printDir(dirStream, currentFile);

    if (closedir(dirStream) == -1) {
        perror("Couldn't close directory");
        return -1;
    }
    return 0;
}
