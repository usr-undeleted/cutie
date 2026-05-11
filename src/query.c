#include "cutie-common.h"
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <bits/posix2_lim.h>

// case sensitive (1) or not (0)
unsigned int beSensitive = 1;
// show lines or not
unsigned int showLines = 0;

void helpMenu(char *invocation) {
    printf("\e[1m%s\e[0m command basic usage:\n"
        "   \e[1m%s\e[0m <flags> <pattern>\n"
        "   \e[3;2mnote that where flags and the pattern are located don't matter; only flags like -e or -A have to be standalone.\e[0m\n"
        "\e[1m%s\e[0m will search stdin and look for a match of your pattern to stdin. you can feed it stdin trough a pipe, for example.\n\n"
        "flags:\n"
        "   \e[1m-h\e[0m or \e[1m--help\e[0m: show this menu.\n"
        "   \e[1m-i\e[0m or \e[1m--ignore-case\e[0m: disable case sensitive searching.\n"
        "   \e[1m-l\e[0m or \e[1m--lines\e[0m: display the line number.\n"
        "   \e[1m-e\e[0m: get anything after flag and treat it as an argument.\n"
        "   \e[1m-A\e[0m: show extra N lines of context after finding match.\n"
        "   \e[1m-B\e[0m: show N lines before match.\n"
        "   \e[1m-C\e[0m: show N extra lines and lines before match of context after finding match.\n\n"
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
        'i',
        'l',
        // keep flags that require one arg here
        // to prevent any errors from labelFlags().
        'e',
        'A',
        'B',
        'C'
    };
    char *stringFlags[] = {
        "--help",
        "--ignore-case",
        "--lines",
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

            if (flags[i] == 1) beSensitive = 0;

            if (flags[i] == 2) showLines = 1;
        }
    } else {
        printf("Invalid flag detected. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
        return 2;
    }

    // context counts; set by flags such as -A
    size_t contextAfter = 0;
    size_t selectedContextAfter = 0;
    size_t contextBefore = 0;
    size_t selectedContextBefore = 0;

    // get extra args; stuff like -e or -A
    // mark array to set stuff to ignore or not
    // 0 = not used (flags or used by -A)
    // 1 = usable (-e, normal)
    int markedArgs[argc];
    for (int i = 0 ; i < argc; i++) {
        if (!i) { markedArgs[i] = 0; continue; };
        markedArgs[i] = 0;

        // isnt a flag?
        if (argv[i][0] != '-') {
            markedArgs[i] = 1;
            continue;
        };

        if (!strcmp(argv[i], "-e")) {
            if (i + 1 < argc) {
                markedArgs[i + 1] = 1;
            } else {
                printf("No argument provided for -e. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
                return 2;
            }
            continue;
        }

        if (!strcmp(argv[i], "-A")) {
            if (i + 1 < argc) {
                markedArgs[i + 1] = 0;
                contextAfter = atoi(argv[i + 1]);
                selectedContextAfter = contextAfter;
                i++; // skip used arg
            } else {
                printf("No argument provided for -A. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
                return 2;
            }
            continue;
        }

        if (!strcmp(argv[i], "-B")) {
            if (i + 1 < argc) {
                markedArgs[i + 1] = 0;
                contextBefore = atoi(argv[i + 1]);
                selectedContextBefore = contextBefore;
                i++; // skip used arg
            } else {
                printf("No argument provided for -B. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
                return 2;
            }
            continue;
        }

        if (!strcmp(argv[i], "-C")) {
            if (i + 1 < argc) {
                markedArgs[i + 1] = 0;
                contextAfter = atoi(argv[i + 1]);
                contextBefore = contextAfter;
                selectedContextAfter = contextAfter;
                selectedContextBefore = contextBefore;
                i++; // skip used arg
            } else {
                printf("No argument provided for -C. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
                return 2;
            }
            continue;
        }
    }

    // find all queries to be matched and their lens
    int allQueries[argc];
    int queryColors[argc];
    size_t colorIdx = 0;
    unsigned int noQuery = 1;
    size_t queriesLen = 0;
    for (int i = 0; i < argc; i++) {
        allQueries[i] = -1;
        queriesLen++;

        if (markedArgs[i] == 1) {
            allQueries[i] = strlen(argv[i]);
            queryColors[i] = colorIdx++;
            noQuery = 0;
        }
    }

    if (noQuery) {
        printf("No query specified. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
        return 2;
    }

    unsigned int onlyFail = 1;
    // normal match
    unsigned int hadMatch = 0;
    // used for context after
    unsigned int doContextAfter = 0;
    // used for context before
    char **beforeBuf = calloc(contextBefore, sizeof(char *));
    size_t bufPos = 0; // where to write
    size_t bufCount = 0; // how many slots are filled
    size_t beforeBufLines[contextBefore];

    char *line = NULL;
    size_t cap = 0;
    ssize_t len;
    // get a valid line
    size_t lastEnd = 0;
    // count the line
    size_t lineNum = 0;
    unsigned int printLineNum = 1;

    while ((len = getline(&line, &cap, stdin)) != -1) {
        lineNum++;
        // context after
        if (doContextAfter) {
            if (showLines) printf("\e[107;30m%zu:\e[0m", lineNum);
            printf("%s", line);
            contextAfter--;
            if (!contextAfter) doContextAfter = 0;
        }
        // store context b4
        if (selectedContextBefore) {
            free(beforeBuf[bufPos]);
            beforeBuf[bufPos] = strdup(line);
            beforeBufLines[bufPos] = lineNum;
            if (selectedContextBefore > 0) {
                bufPos = (bufPos + 1) % contextBefore;
            };
            if (bufCount < contextBefore) bufCount++;
        }

        lastEnd = 0;
        hadMatch = 0;
        // walk trough stdin
        for (int i = 0; i < len; i++) {

            // lets us pick the first longest query
            int bestIdx = -1;
            size_t bestLen = 0;

            // walk trough all queries provided
            for (int j = 0; j < queriesLen; j++) {
                if (allQueries[j] == -1) continue;

                // if theres a match, we set the query to be shown as
                // the longest one
                int compVal = 1;
                if (beSensitive) {
                    compVal = strncmp(line + i, argv[j], allQueries[j]);
                } else {
                    compVal = strncasecmp(line + i, argv[j], allQueries[j]);
                }

                if (!compVal) {
                    if (allQueries[j] > bestLen) {
                        bestLen = allQueries[j];
                        bestIdx = j;
                    }
                }
            }

            if (bestIdx != -1) {
                // context b4
                size_t start = (bufCount < contextBefore) ? 0 : bufPos;
                for (size_t k = 0; k < bufCount; k++) {
                    size_t idx = selectedContextBefore > 0 ? (start + k) % contextBefore: 0;
                    if (showLines && beforeBufLines[idx] != lineNum) {
                        printf("\e[107;30m%zu:\e[0m", beforeBufLines[idx]);
                    }
                    if (beforeBufLines[idx] == lineNum) continue; // skip matched line
                    printf("%s", beforeBuf[idx]);
                }

                if (showLines) printf("\e[107;30m%zu:\e[0m", lineNum);
                printLineNum = 0;
                printf("%.*s", (int)(i - lastEnd), line + lastEnd);
                printf("\e[3%d;1m%.*s\e[0m", (queryColors[bestIdx] + 1) % 6 + 1, (int)bestLen, line + i);
                lastEnd = i + bestLen;
                i += bestLen - 1;
                onlyFail = 0;
                hadMatch = 1;

                bufCount = 0;
                bufPos = 0;
                doContextAfter = selectedContextAfter > 0 ? 1 : 0;
                contextAfter = selectedContextAfter;
            }
        }
        if (hadMatch) {
            printf("%s", line + lastEnd);
        }

        printLineNum = 1;
    }
    free(line);

    return onlyFail ? 1 : 0;
}
