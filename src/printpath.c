#include <asm-generic/errno-base.h>
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
    unsigned int singleDir;

    if (argc <= 2) singleDir = 1;

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
            if ((realpath(argv[i], dir)) == NULL) {
                printf("Couldn't get realpath for '%s': %s\n", argv[i], strerror(errno));
                if (!singleDir) {
                    continue;
                } else {
                    return 2;
                }
            }

            dirStream = opendir(dir);

            if (dirStream != NULL || errno == ENOTDIR) {
                printf("%s\n", dir);
            } else {
                printf("Couldn't check '%s': %s\n", dir, strerror(errno));
                if (!singleDir) {
                    continue;
                } else {
                    return 1;
                }
            }
        }
    }

    return 0;
}
