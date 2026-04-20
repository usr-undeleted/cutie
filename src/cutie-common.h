#include <stdlib.h>
#include <string.h>

// return int array that contains int values for each argv that starts with '-' or '--'
// return NULL on error
struct flagInput {
    char *charFlags;
    int charLen;
    char **stringFlags;
    int stringLen;
    size_t flagCount;
};

int *labelFlags(int argc, char *argv[], struct flagInput *input) {

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
        input->flagCount = 0;
        return (int*)malloc(sizeof(int));
    }

    int *returned = (int*)malloc(size * sizeof(int));
    int index = 0;

    for (int i = 0; i < argc; i++) {
        int matched = 0;
        if (argv[i][0] == '-') {

            if (argv[i][1] == '-') {
                for (int j = 0; j < input->stringLen; j++) {
                    if (!strcmp(argv[i], input->stringFlags[j])) {
                        returned[index] = j;
                        index++;
                        matched = 1;
                        break;
                    }
                }

                if (!matched) return NULL;

            } else {
                for (int j = 1; j < strlen(argv[i]); j++) {
                    int matched = 0;
                    for (int k = 0; k < input->charLen; k++) {
                        if (argv[i][j] == input->charFlags[k]) {
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

    input->flagCount = (size_t)index;
    return returned;
}

// return color depending on file extension
// return null on fail, caller should see
// null and use their own fallback
//
// use color or not
int useColor = 0;
char *determineColor(const char *filename) {
    if (!useColor) {
        return NULL;
    }

    char *lsColors = getenv("LS_COLORS"); // NEVER use this
    if (!lsColors) {
        return NULL;
    }

    const char *extension = strrchr(filename, '.');
    if (!extension) {
        return NULL;
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
        return NULL;
    }
}
