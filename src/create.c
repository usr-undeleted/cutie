#include "cutie-common.h"
#include <asm-generic/errno-base.h>
#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

void helpMenu(char *invocation) {
    printf("\e[1m%s\e[0m command basic usage:\n"
        "   \e[1m%s\e[0m <files>n"
        "   \e[3;2mnote that the order of flags and files dont matter.\e[0m\n"
        "   %s will create any files/dirs specified, if they don't exist.\n"
        "   if you specify a '/' in the end of the argument, %s will create a folder. \n   \"%s dir/dir/file\" will create the path necessary for that file's creation.\n"
        "\e[1m%s\e[0m will, well, create any files/dirs you specify.\n\n"
        "flags:\n"
        "   \e[1m-h\e[0m or \e[1m--help\e[0m: show this menu.\n\n"
        "\e[2;3m%s is part of the cutie project hosted under https://github.com/usr-undeleted/cutie licensed under the GPLv3 license.\e[0m\n",
        invocation, invocation, invocation, invocation, invocation, invocation, invocation
    );
    exit(0);
}

int main (int argc, char *argv[]) {
    int errorCode = 0;

    // manage flags
    // we dont use labelflags here cus like, why
    // malloc for a single flag???
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') continue;

        if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            helpMenu(argv[0]);
        } else {
            fprintf(stderr, "Invalid flag detected. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
            return 2;
        }
    }


    // create (heh)
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "/")) {
            fprintf(stderr, "Dir can't be created: name is invalid.\n");
            errorCode = 2;
            break;
        }

        char path[PATH_MAX];
        strcpy(path, argv[i]);

        unsigned int isDir = 0;
        unsigned int onlyBar = 1;
        size_t len = strlen(path);
        if (path[len] == '/') isDir = 1;
        // something like "//"

        for (char *p = path + 1; *p; p++) {
            if (*p == '/') {
                *p = '\0';
                mkdir(path, EEXIST);
                *p = '/';
            } else {
                onlyBar = 0;
            }
        }

        if (onlyBar) {
            fprintf(stderr, "Dir can't be created: name is invalid.\n");
            errorCode = 2;
            break;
        }

        int ret;
        if (isDir) ret = mkdir(path, EEXIST);
        else ret = open(path, O_CREAT | O_EXCL, 0644);

        if (ret == -1) {
            fprintf(stderr, "%s couldn't be created: %s\n", isDir ? "Directory" : "File", strerror(errno));
            errorCode = 2;
            break;
        }

        if (!isDir) close(ret);
    }
    return errorCode;
}
