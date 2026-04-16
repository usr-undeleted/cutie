#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

// array with pointers to structs

int main (int argc, char *argv[]) {
    char *dir;
    DIR *dirStream;
    struct dirent *currentFile;

    if (argc == 1) { // get cwd
        dirStream = opendir(".");
    } else {
        dirStream = opendir(argv[1]);
    }

    if (dirStream == NULL) {
        perror("Directory couldn't be opened");
        return 1;
    }

    // print directory
    int quant = 0;
    while ((currentFile = readdir(dirStream)) != NULL) {

        if (quant == 7) {
            printf("\n");
            quant = 0;
        }

        if (currentFile->d_name[0] != '.') {
            if (currentFile->d_type == DT_DIR) {
                printf("\033[34m%s\033[0m   ",currentFile->d_name);
            } else {
                printf("%s\033[0m  ",currentFile->d_name);
            }

            quant++;
        }
    }
    if (quant != 7) {
        printf("\n");
    }

    if (closedir(dirStream) == -1) {
        perror("Couldn't close directory");
        return -1;
    }
    return 0;
}
