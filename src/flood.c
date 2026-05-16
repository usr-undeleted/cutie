#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define DEFAULT_UNIT "y\n"
// while 32768 is more efficient, we need to be mindful of people who dont
// have much cache in their cpus. inclusion forever!
#define FLOOD_BUFSIZE 16384

int main (int argc, char *argv[]) {
    char *buffer;
    size_t bufSize;
    char *unit;
    size_t unitLen = 0;

    if (argc == 1) {
        // default to 'y\n'
        unitLen = strlen(DEFAULT_UNIT);
        unit = DEFAULT_UNIT;

    } else {
        // count len to be malloced
        for ( int i = 1; i < argc; i++) {
            unitLen += strlen(argv[i]) + 1;
        }
        // concatenate
        unit = (char *)malloc(unitLen);
        int idx = 0;
        for (int i = 1; i < argc; i++) {
            // copy word over and advance
            size_t len = strlen(argv[i]);
            memcpy(unit + idx, argv[i], len);
            idx += len;
            // put space and advance
            unit[idx] = ' ';
            idx++;
            // set last char to '\n'
            if (i == (argc - 1)) unit[idx - 1] = '\n';
        }
    }

    // fill buffer with as much as possible of unit
    buffer = (char *)malloc(FLOOD_BUFSIZE);
    size_t bufUsed = 0;
    // fill up buffer as much as possible
    while (bufUsed + unitLen <= FLOOD_BUFSIZE) {
        memcpy(buffer + bufUsed, unit, unitLen);
        // how much space we used
        bufUsed += unitLen;
    }

    // flooooooooddddd!!!
    while (fwrite(buffer, 1, bufUsed, stdout) == bufUsed);
    // incase we do actually escape
    if (argc != 1) free(unit);
    return 0;
}
