#include "cutie-common.h"
#include <unistd.h>

void helpMenu(char *invocation) {
    printf("\e[1m%s\e[0m command basic usage:\n"
        "   \e[1m%s\e[0m <flags> <pattern>\n"
        "   \e[3;2mnote that where flags and the pattern are located don't matter.\e[0m\n"
        "\e[1m%s\e[0m will search stdin and look for a match of your pattern to stdin. you can feed it stdin trough a pipe, for example.\n\n"
        "flags:\n"
        "   \e[1m-h\e[0m or \e[1m--help\e[0m: show this menu.\n"
        "\e[2;3m%s is part of the cutie project hosted under https://github.com/usr-undeleted/cutie licensed under the GPLv3 license.\e[0m\n",
        invocation, invocation, invocation, invocation
    );
    exit(0);
}

// always read from stdin. if the user wants to read a
// file, they shall pipe it
int main (int argc, char *argv[]) {
    // manage empty stdin (no pipe)
    if (isatty(STDIN_FILENO)) {
        printf("Stdin wasn't provided. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
        return 2;
    }

    // manage flags
    char charFlags[] = {
        'h'
    };
    char *stringFlags[] = {
        "--help"
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
        }
    } else {
        printf("Invalid flag detected. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
        return 2;
    }

    // find what the user wants to query
    unsigned int queryPos = -1;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            queryPos = i;
            break;
        }
    }
    if (queryPos == -1) {
        printf("No query specified. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
        return 2;
    }
    size_t queryLen = strlen(argv[queryPos]);

    unsigned int onlyFail = 1;
    unsigned int hadMatch = 0;
    char *line = NULL;
    size_t cap = 0;
    ssize_t len;
    // get a valid line
    size_t lastEnd = 0;
    while ((len = getline(&line, &cap, stdin)) != -1) {
        lastEnd = 0;
        hadMatch = 0;

        // go trough all of line to find a matching char
        for (int i = 0; i < len; i++) {
            if (line[i] == argv[queryPos][0]) {

                // compare
                if (!strncmp(line + i, argv[queryPos], queryLen)) {
                    // we have a match!
                    printf("%.*s", (int)(i - lastEnd), line + lastEnd); // anything before the match
                    printf("\e[31;1m%.*s\e[0m", (int)queryLen, line + i); // the actual query
                    lastEnd = i + queryLen; // set when to start printing again
                    i += queryLen - 1; // remove the match from the printing
                    onlyFail = 0;
                    hadMatch = 1;
                }
            }
        }
        if (hadMatch) {
            printf("%s", line + lastEnd);
        }
    }
    free(line);

    return onlyFail ? 1 : 0;
}
