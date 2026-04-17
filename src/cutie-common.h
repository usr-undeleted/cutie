#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// tolower in strings
char *strToLower(char *string) {
    for (int i = 0; i < strlen(string); i++) {
        string[i] = tolower(string[i]);
    }
}

// label flags used, return NULL if failed
// returns int array to show all flags used; only check '-' or '--'
// -1 = unset, not hypen
// 0 = help menu, always
char charFlags[] = {
    'h'
    // indicate where a flag is used if it isnt in every func:
    // -b // used in: foo, bar
};
char *stringFlags[] = {
    "--help"
};

int *labelFlags(int argc, char *argv[]) {
    int *returned = (int*)malloc((size_t)argc);
    int charLen = sizeof(charFlags) / sizeof(charFlags[0]);
    int stringLen = sizeof(stringFlags) / sizeof(stringFlags[0]);
    int fail = 0;

    for (int i = 0; i < argc; i++) {
        returned[i] = -1;
        if (argv[i][0] == '-') {
            if (argv[i][1] == '-') { // --
                for (int j = 0; j < stringLen; j++) {
                    if (!strcmp(argv[i], stringFlags[j])) {
                        returned[i] = j;
                    } else {
                        fail = 1;
                    }
                }
                continue;
            }

            for (int j = 0; j < charLen; j++) { // -
                if (argv[i][1] == charFlags[j]) {
                    returned[i] = j;
                } else {
                    fail = 1;
                }
            }

        }
    }

    if (!fail) {
        return returned;
    } else {
        return NULL;
    }
}

void helpMenu() {
    printf("scan command basic usage:\n"
        "   scan <flags> <dirs>\n"
        "scan will search trough all directories you specify.\n"
        "flags:\n"
        "   -h or --help: show this menu.\n\n"
        "scan is part of the cutie project hosted under https://github.com/usr-undeleted/cutie licensed under the GPLv3 license.\n"
    );
    exit(0);
}
