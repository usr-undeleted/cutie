#include "cutie-common.h"
#include <asm-generic/errno-base.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

void helpMenu(char *invocation) {
    printf("\e[1m%s\e[0m command basic usage:\n"
        "   \e[1m%s\e[0m <flags> <file(s)>\n"
        "   \e[3;2mnote that the order of flags and dirs dont matter.\e[0m\n"
        "   use '%s -' to specificy stdin from -, as long as it is provided.\n"
        "\e[1m%s\e[0m will print the contents all files you specify.\n\n"
        "flags:\n"
        "   \e[1m-h\e[0m or \e[1m--help\e[0m: show this menu.\n\n"
        "\e[2;3m%s is part of the cutie project hosted under https://github.com/usr-undeleted/cutie licensed under the GPLv3 license.\e[0m\n",
        invocation, invocation, invocation, invocation, invocation
    );
    exit(0);
}

// return 1 on fail
int dump(FILE *fd) {
    char buf[4096];
    size_t n;
    int returned = 0;

    // dump
    while ((n = fread(buf, 1, sizeof(buf), fd)) > 0) {
        if (fwrite(buf, 1, n, stdout) < n) {
            returned = 1;
        }
    }

    // handle invalid fd
    if (ferror(fd)) {
        returned = 1;
    }

    return returned;
}

int main (int argc, char *argv[]) {
    FILE *fileDescriptor;
    // toggles wether or not to proccess stdin
    unsigned int processStdin = 0;
    // return code
    int returned = 0;

    // do we process stdin?
    if (!isatty(STDIN_FILENO)) {
        processStdin = 1;
    }

    // handle stdin when alone
    if (argc == 1) {
        if (!processStdin) {
            printf("Not enough arguments given. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
            return 2;
        } else {
            returned = dump(stdin);
        }
    }

    // use dump() to print
    for (int i = 1; i < argc; i++) {
        // handle flags
        if (argv[i][0] == '-' && argv[i][1] != '\0') continue;

        if (argv[i][0] == '-' && argv[i][1] == '\0' && processStdin) {
            returned = dump(stdin);
            continue;
        }

        int fd = open(argv[i], O_RDONLY);
        if (fd == -1) {
            printf("Failed to read file '%s': %s\n", argv[i], strerror(errno));
            if (argc == 1) {
                return 2;
            } else {
                continue;
            }
        }
        // is dir?
        struct stat st;
        fstat(fd, &st);
        if (S_ISDIR(st.st_mode)) {
            // the fake errno of doom
            printf("Failed to read file '%s': Is a directory\n", argv[i]);
            close(fd);
            if (argc == 1) {
                return 2;
            } else {
                continue;
            }
        }

        fileDescriptor = fdopen(fd, "r");
        returned = dump(fileDescriptor);
    }

    return returned;
}
