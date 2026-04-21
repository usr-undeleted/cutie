#include <stdio.h>
#include <stdlib.h>

/*
 * Simple printf wrapper.
 * Usage: print [TEXT]...
 * Prints the arguments separated by spaces followed by a newline.
 * If no arguments are given, prints nothing.
 */
int main(int argc, char *argv[]) {
    if (argc <= 1) {
        return 0;
    }
    for (int i = 1; i < argc; ++i) {
        fputs(argv[i], stdout);
        if (i < argc - 1) {
            fputc(' ', stdout);
        }
    }
    fputc('\n', stdout);
    return 0;
}
