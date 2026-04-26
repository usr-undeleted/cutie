#include "cutie-common.h"
#include <stdlib.h>
#include <unistd.h>

// case sensitive (1) or not (0)
unsigned int beSensitive = 1;

void helpMenu(char *invocation) {
    printf("\e[1m%s\e[0m command basic usage:\n"
        "   \e[1m%s\e[0m <flags> <pattern>\n"
        "   \e[3;2mnote that where flags and the pattern are located don't matter.\e[0m\n"
        "\e[1m%s\e[0m will search stdin and look for a match of your pattern to stdin. you can feed it stdin trough a pipe, for example.\n\n"
        "flags:\n"
        "   \e[1m-h\e[0m or \e[1m--help\e[0m: show this menu.\n"
        "   \e[1m-i\e[0m or \e[1m--ignore-case\e[0m: disable case sensitive searching.\n\n"
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
        'h',
        'i'
    };
    char *stringFlags[] = {
        "--help",
        "--ignore-case"
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

    // find all queries to be matched and their lens
    int allQueries[argc];
    unsigned int noQuery = 1;
    size_t queriesLen = 0;
    for (int i = 0; i < argc; i++) {
        allQueries[i] = -1;
        queriesLen++;

        if (i != 0 && argv[i][0] != '-') {
            allQueries[i] = strlen(argv[i]);
            noQuery = 0;
        }
    }

    if (noQuery) {
        printf("No query specified. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
        return 2;
    }

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
        // walk trough stdin
        for (int i = 0; i < len; i++) {

            // lets us pick the first longest query
            int bestIdx = -1;
            size_t bestLen = 0;

            // walk trough all queries provided
            for (int j = 0; j < queriesLen; j++) {
                if (line[i] != argv[j][0] || allQueries[j] == -1) continue;

                // if theres a match, we set the query to be shown as
                // the longest one
                if (!strncmp(line + i, argv[j], allQueries[j])) {
                    if (allQueries[j] > bestLen) {
                        bestLen = allQueries[j];
                        bestIdx = j;
                    }
                }
            }

            if (bestIdx != -1) {
                printf("%.*s", (int)(i - lastEnd), line + lastEnd);
                printf("\e[3%d;1m%.*s\e[0m", bestIdx % 7, (int)bestLen, line + i);
                lastEnd = i + bestLen;
                i += bestLen - 1;
                onlyFail = 0;
                hadMatch = 1;
            }
        }
        if (hadMatch) {
            printf("%s", line + lastEnd);
        }
    }
    free(line);

    return onlyFail ? 1 : 0;
}
