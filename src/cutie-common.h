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

// returns "0" on fallbacks and fails
// finds the proper color for the file type
// edits colorLen to tell the caller when to
// stop printing, returns the offset of lsColors
//
// use color or not
unsigned int useColor = 0;
// contains offsets and limits for strings on
// lsColors, indicating the extension and its
// color code
struct colorEntry {
    size_t extStart;
    size_t extEnd;
    size_t colorStart;
    size_t colorEnd;
};
struct colorEntry *parsedColors;
size_t parsedColorCount;
const char *lsColors;

char *determineColor(struct entry entry, struct stat *st, size_t *colorLen) {
    *colorLen = 1;
    if (!useColor || !lsColors || st == NULL) {
        return "0";
    }

    size_t start, end;
    unsigned int hadMatch = 0;

    if (entry.type == DT_DIR) {
        *colorLen = 4;
        return "34;1";

    } else if (entry.type == DT_LNK) {
        *colorLen = 4;
        return "36;1";

    } else if (S_ISREG(st->st_mode)
        && (st->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
        && entry.type != DT_DIR) {
            *colorLen = 4;
        return "32;1";

    } else if (entry.type == DT_BLK || entry.type == DT_CHR) {
        *colorLen = 7;
        return "40;33;1";

    } else if (entry.type == DT_SOCK) {
        *colorLen = 4;
        return "0;35";

    } else if (entry.type == DT_FIFO) {
        *colorLen = 4;
        return "0;33";

    } else if (entry.type == DT_UNKNOWN || entry.type == DT_REG) {
        const char *extension = strrchr(entry.name, '.');
        if (!extension) {
            return "0";
        }

        // find the proper entry in the caller-made
        // colorEntry array
        for (size_t i = 0; i < parsedColorCount; i++) {
            // find matching extension
            if (!strncmp(extension, lsColors + parsedColors[i].extStart, parsedColors[i].extEnd - parsedColors[i].extStart)) {
                *colorLen = parsedColors[i].colorEnd - parsedColors[i].colorStart;
                return (char *)lsColors + parsedColors[i].colorStart;
            }
        }
        // if theres no match
        return "0";

    } else {
        // fallback
        *colorLen = 1;
        return "0";
    }
}
