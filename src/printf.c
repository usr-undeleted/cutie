#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        return 0;
    }
    for (char *p = argv[1]; *p; p++) {
        if (*p == '\\' && *(p+1)) {
            p++;
            switch (*p) {
                case 'n': putchar('\n'); break;
                case 't': putchar('\t'); break;
                case 'e': putchar('\e'); break;

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

    return 0;
}
