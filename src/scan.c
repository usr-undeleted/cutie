#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

// will contain a whole directory
struct entry {
    char *name;
    unsigned char type; // DT_DIR, DT_REG, etc
};

int cmpEntries(const void *a, const void *b) {
    return strcmp(((struct entry *)a)->name, ((struct entry *)b)->name);
}

void printDir(DIR *dirStream, struct dirent *currentFile) {
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

        //if (dirFileCount % 6 == 0) {
        //    printf("\n");
        //}

        entries[dirFileCount].name = currentFile->d_name;
        entries[dirFileCount].type = currentFile->d_type;

        dirFileCount++;
    }

    qsort(entries, dirFileCount, sizeof(struct entry), cmpEntries);

    for (int i = 0; i < dirFileCount; i++) {
        printf("%-*s\033[0m", (int)largestWordSize + 1, entries[i].name);
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
