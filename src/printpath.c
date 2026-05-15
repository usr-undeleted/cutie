#include <asm-generic/errno-base.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

void helpMenu(char *invocation) {
    printf("\e[1m%s\e[0m command basic usage:\n"
        "   \e[1m%s\e[0m <files>\n"
        "\e[1m%s\e[0m will make the real path for all files or directories specified.\n\n"
        "flags:\n"
        "   \e[1m-h\e[0m or \e[1m--help\e[0m: show this menu.\n\n"
        "\e[2;3m%s is part of the cutie project hosted under https://github.com/usr-undeleted/cutie licensed under the GPLv3 license.\e[0m\n",
        invocation, invocation, invocation, invocation
    );
    exit(0);
}

int main (int argc, char *argv[]) {
    char dir[PATH_MAX + 1];
    struct dirent *currentFile;
    DIR *dirStream;
    unsigned int singleDir;

    // manage flags
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') continue;

        if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            helpMenu(argv[0]);
        } else {
            fprintf(stderr, "Invalid flag detected. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
            return 2;
        }
    }

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
                fprintf(stderr, "Couldn't get realpath for '%s': %s\n", argv[i], strerror(errno));
                if (!singleDir) {
                    continue;
                } else {
                    return 2;
                }
            }

            dirStream = opendir(dir);

            if (dirStream != NULL || errno == ENOTDIR || errno != EPERM) {
                printf("%s\n", dir);
            } else {
                fprintf(stderr, "Couldn't check '%s': %s\n", dir, strerror(errno));
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
