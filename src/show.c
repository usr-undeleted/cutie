#include "cutie-common.h"
#include <asm-generic/errno-base.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

// invisible chars
unsigned int nonPrinting = 0;
unsigned int eofChar = 0;
unsigned int tabChar = 0;

void helpMenu(char *invocation) {
    printf("\e[1m%s\e[0m command basic usage:\n"
        "   \e[1m%s\e[0m <flags> <file(s)>\n"
        "   \e[3;2mnote that the order of flags and dirs dont matter.\e[0m\n"
        "   use '%s -' to specify stdin from -, as long as it is provided.\n"
        "\e[1m%s\e[0m will print the contents all files you specify.\n\n"
        "flags:\n"
        "   \e[1m-h\e[0m or \e[1m--help\e[0m: show this menu.\n"
        "   \e[1m-v\e[0m or \e[1m--show-nonprinting\e[0m: use ^ and M- for hidden chars not normally printed.\n"
        "   \e[1m-T\e[0m or \e[1m--show-tabs\e[0m: show tab characters as ^I.\n"
        "   \e[1m-E\e[0m or \e[1m--show-eof\e[0m: show EOFs as $.\n"
        "\e[2;3m%s is part of the cutie project hosted under https://github.com/usr-undeleted/cutie licensed under the GPLv3 license.\e[0m\n",
        invocation, invocation, invocation, invocation, invocation
    );
    exit(0);
}

// called by dump() to handle invisible stuff n all
int dumpVisible(FILE *fd) {
    char buf[4096];
    size_t n;
    int returned = 0;

    while ((n = fread(buf, 1, sizeof(buf), fd)) > 0) {
        for (size_t i = 0; i < n; i++) {
            unsigned char ch = buf[i];

            if (eofChar && ch == '\n') {
                printf("$\n");
                continue;
            }

            if (tabChar && ch == '\t') {
                printf("^I");
                continue;
            }

            if (nonPrinting) {
                if (ch <= 31 && ch >= 0 && ch != '\n' && ch != '\t') {
                    printf("^%c", ch + '@');
                    continue;
                }

                if (ch == 127) {
                    printf("^?");
                    continue;
                }

                if (ch <= 255 && ch >= 128) {
                    printf("M-%c", ch - 128);
                    continue;
                }
            }

            // regular printing
            putchar(ch);
        }
    }

    return returned;
}

// return 1 on fail
int dump(FILE *fd) {
    char buf[4096];
    size_t n;
    int returned = 0;

    if (nonPrinting || eofChar || tabChar) return dumpVisible(fd);

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

    // manage flags
    char charFlags[] = {
        'h',
        'v',
        'T',
        'E'
    };
    char *stringFlags[] = {
        "--help",
        "--show-nonprinting"
        "--show-tabs",
        "--show-eof"
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
            if (flags[i] == 0) helpMenu(argv[0]);

            if (flags[i] == 1) nonPrinting = 1;

            if (flags[i] == 2) tabChar = 1;

            if (flags[i] == 3) eofChar = 1;
        }
    } else {
        fprintf(stderr, "Invalid flag detected. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
        return 2;
    }
    free(flags);

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
