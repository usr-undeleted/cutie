#include <linux/limits.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

// will contain a whole directory
struct entry {
    char name[NAME_MAX + 1];
    unsigned char type; // DT_DIR, DT_REG, etc
};

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
    if (returned == NULL) {
        printf("Failed to get flags; failed malloc.\n");
        exit(2);
    }
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

// use color or not
unsigned int useColor = 0;
// contains lscolors, determined by caller
char *colors;

char *determineColor(struct entry entry, struct stat *st, unsigned int *needsFree) {
    if (!useColor || !colors || st == NULL) {
        *needsFree = 0;
        return "0";
    }
    const char *extension = strrchr(entry.name, '.');

    size_t start, end;
    unsigned int hadMatch = 0;

    if (entry.type == DT_DIR) {
        *needsFree = 0;
        return "34;1";

    } else if (entry.type == DT_LNK) {
        *needsFree = 0;
        return "36;1";

    } else if (S_ISREG(st->st_mode)
        && (st->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
        && entry.type != DT_DIR) {
        *needsFree = 0;
        return "32;1";

    } else if (entry.type == DT_BLK || entry.type == DT_CHR) {
        *needsFree = 0;
        return "40;33;1";

    } else if (entry.type == DT_SOCK) {
        *needsFree = 0;
        return "0;35";

    } else if (entry.type == DT_FIFO) {
        *needsFree = 0;
        return "0;33";

    } else if (entry.type == DT_UNKNOWN || entry.type == DT_REG) {
        if (!extension) {
            *needsFree = 0;
            return "0";
        }

        for (int i = 0; i < strlen(colors); i++) {
            if (!strncmp(colors + i, extension, strlen(extension))) {
                // we have a match
                hadMatch = 1;
                start = i;
                end = start;
            }

            if (hadMatch) {
                // shift start
                if (colors[i] == '=') {
                    start = i + 1;
                }

                // stop the search
                if (colors[i] != ':') {
                    end++;
                } else {
                    break;
                }
            }
        }

        // resolved is the color code
        char *resolved;
        if (hadMatch) {
            resolved = (char*)malloc(end - start + 1);
            if (resolved == NULL) {
                printf("Color couldn't be determined; failed malloc.\n");
                exit(2);
            }

        } else {
            *needsFree = 0;
            return "0";
        }
        resolved[end - start] = '\0';
        for (size_t i = 0; i < (end - start); i++) {
            resolved[i] = colors[i + start];
        }

        if (resolved) {
            *needsFree = 1;
            return resolved;
        } else {
            *needsFree = 0;
            return "0";
        }

    } else {
        // fallback
        *needsFree = 0;
        return "0";
    }
}
