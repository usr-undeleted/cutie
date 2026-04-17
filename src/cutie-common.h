#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// label flags used, return NULL if failed
// returns int array to show all flags used; only check '-' or '--'
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

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == '-') { // --
                for (int j = 0; j < stringLen; j++) {
                    if (!strcmp(argv[i], stringFlags[j])) {
                        returned[i] = j;
                    } else {
                        returned[i] = -1;
                        fail = 1;
                    }
                }
                continue;
            }

            for (int j = 0; j < charLen; j++) { // -
                if (argv[i][1] == charFlags[j]) {
                    returned[i] = j;
                } else {
                    returned[i] = -1;
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
        "scan will search trough all directories you specificy.\n"
        "flags:\n"
        "   -h or --help: show this menu.\n\n"
        "scan is part of the cutie project hosted under https://github.com/usr-undeleted/cutie licensed under the GPLv3 license."
    );
    exit(0);
}
