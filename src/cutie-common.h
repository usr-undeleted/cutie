#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// tolower in strings
void strToLower(char *string) {
    for (int i = 0; i < strlen(string); i++) {
        string[i] = tolower(string[i]);
    }
}

int *labelFlags(int argc, char *argv[], char *charFlags, int charLen,  char **stringFlags, int stringLen) {
    int *returned = (int*)malloc((size_t)argc);
    int fail = 0;
    int match = 0;

    for (int i = 0; i < argc; i++) {
        returned[i] = -1;
        match = 0;
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
                    match = 1;
                    break;
                }
            }
            if (!match) {
                fail = 1;
            }

        }
    }

    if (!fail) {
        return returned;
    } else {
        return NULL;
    }
}
