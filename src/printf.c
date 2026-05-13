#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void print(char *str) {
    for (char *p = str; *p; p++) {
        if (*p == '\\' && *(p+1)) {
            p++;
            switch (*p) {
                case 'n': putchar('\n'); break;
                case 't': putchar('\t'); break;
                case 'e': putchar('\e'); break;
                case 'a': putchar('\a'); break;
                case 'r': putchar('\r'); break;
                case 'v': putchar('\v'); break;
                case 'f': putchar('\f'); break;
                case '"': putchar('"'); break;
                case 'c': exit(0);

                case '0': case '1': case '2': case '3':
                case '4': case '5': case '6': case '7': {
                    int val = *p - '0';
                    if (p[1] >= '0' && p[1] <= '7') { val = val * 8 + (p[1] - '0'); p++; }
                    if (p[1] >= '0' && p[1] <= '7') { val = val * 8 + (p[1] - '0'); p++; }
                    putchar(val);
                    break;
                }

                case '\\': putchar('\\'); break;
                default: putchar('\\'); putchar(*p);
            }
        } else {
            putchar(*p);
        }
    }
}

int main(int argc, char *argv[]) {
    unsigned int piped = 0;
    if (!isatty(STDIN_FILENO)) piped = 1;

    if (argc <= 1 && !piped) {
        printf("Not enough arguments given. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
        return 2;
    }

    if (piped) {
        char buf[4096];
        size_t n;

        while ((n = fread(buf, 1, sizeof(buf), stdin)) > 0) {
            buf[n] = '\0';
            print(buf);
        }

        if (argc == 1) return 0;
    }

    print(argv[1]);

    return 0;
}
