#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int main (int argc, char *argv[]) {
    char dir[PATH_MAX + 1];
    struct dirent *currentFile;
    DIR *dirStream;

    if (argc == 1) {
        dirStream = opendir(".");

        if (getcwd(dir, sizeof(dir)) != NULL) {
               // copied absolute cwd to dir
        } else {
               perror("Couldn't get absolute of current dir");
               return 1;
        }

        printf("%s\n", dir);
        closedir(dirStream);

    } else {
        for (int i = 1; i < argc; i++) {
            realpath(argv[i], dir);
            dirStream = opendir(dir);

            if (dirStream != NULL) {
                printf("%s\n", realpath(argv[i], dir));
            } else {
                printf("Couldn't check '%s': %s\n", dir, strerror(errno));
            }
        }
        closedir(dirStream);
    }

    return 0;
}
