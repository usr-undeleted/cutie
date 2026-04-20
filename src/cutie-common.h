#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// tolower in strings
void strToLower(char *string) {
    for (int i = 0; i < strlen(string); i++) {
        string[i] = tolower(string[i]);
    }
}

// return int array that contains int values for each argv that starts with '-' or '--'
// return NULL on error
int *labelFlags(int argc, char *argv[], char *charFlags, int charLen,  char **stringFlags, int stringLen, size_t *flagCount) {

    // get malloc size
    size_t size = 0;
    int hasFlag = 0;
    for (int i = 0; i < argc; i++) {
        if (strlen(argv[i]) > 1 && argv[i][0] == '-') {
            if (argv[i][1] == '-') {
                hasFlag = 1;
                size++;
            } else {
                hasFlag = 1;
                size += strlen(argv[i]) - 1;
            }
        }
    }

    if (size <= 0 && hasFlag) {
        return NULL;
    }

    if (size == 0) {
        *flagCount = 0;
        return (int*)malloc(sizeof(int));
    }

    int *returned = (int*)malloc(size *sizeof(int));

    // loop trough flag args
    int index = 0; // returned index

    for (int i = 0; i < argc; i++) {
        returned[i] = -1;
        if (argv[i][0] == '-') {

            // if its a full word flag
            if (argv[i][1] == '-') {
                for (int j = 0; j < stringLen; j++) {
                    if (!strcmp(argv[i], stringFlags[j])) {
                        returned[index] = j;
                        index++;
                        break;
                    }
                }

            // if its just a '-thing'
            } else {
                for (int j = 1; j < strlen(argv[i]); j++) {
                    int matched = 0;
                    for (int k = 0; k < charLen; k++) {
                        if (argv[i][j] == charFlags[k]) {
                            returned[index] = k;
                            index++;
                            matched = 1;
                            break;
                        }
                    }
                    if (!matched) return NULL;
                }
            }
        }
    }

    *flagCount = (size_t)index;
    return returned;
}

// return color depending on file extension
char *determineColor(const char *filename) {
    char *lsColors = getenv("LS_COLORS"); // NEVER use this
    char *normal = strdup("0");
    if (!lsColors) {
        return normal;
    }

    const char *extension = strrchr(filename, '.');
    if (!extension) {
        return normal;
    }

    char *colors = strdup(lsColors);
    char *token = strtok(colors, ":");
    char *colorCode = NULL;

    while (token != NULL) {
        if (token[0] == '*' && strncmp(token + 1, extension, strlen(extension)) == 0) {
            colorCode = strchr(token, '=');
            if (colorCode) {
                colorCode++;
                break;
            }
        }
        token = strtok(NULL, ":");
    }

    free(colors);
    if (colorCode) {
        return colorCode;
    } else {
        return normal;
    }
}
