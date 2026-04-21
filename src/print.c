#include <stdio.h>
#include <stdlib.h>

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
