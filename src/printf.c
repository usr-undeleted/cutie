#include "cutie-common.h"
#include <ctype.h>
#include <stdio.h>

void helpMenu(char *invocation) {
    printf("\e[1m%s\e[0m command basic usage:\n"
        "   \e[1m%s\e[0m <input>\n"
        "   stdin will be processed along the first arg, if provided.\n"
        "\e[1m%s\e[0m will print the escape characters and content in first arg. does not format.\n\n"
        "flags:\n"
        "   \e[1m-h\e[0m or \e[1m--help\e[0m: show this menu.\n\n"
        "escape chars:\n"
        "   \e[1m\\n\e[0m prints a new line.\n"
        "   \e[1m\\t\e[0m prints a new tab.\n"
        "   \e[1m\\b\e[0m prints a vertical tab.\n"
        "   \e[1m\\\"\e[0m prints a '\"'.\n"
        "   \e[1m\\\\\e[0m prints a '\\'\n"
        "   \e[1m\\b\e[0m backspaces.\n"
        "   \e[1m\\e\e[0m is escape.\n"
        "   \e[1m\\a\e[0m plays the terminal's bell.\n"
        "   \e[1m\\c\e[0m prevents any input beyond it.\n"
        "   \e[1m\\r\e[0m returns cursor to start of line.\n"
        "   \e[1m\\f\e[0m is form feed.\n\n"
        "formatting (%%...):\n"
        "\e[37;3myou must provide one or more arguments for every format used.\e[0m\n"
        "   \e[1m%%s\e[0m prints the argument as a string.\n"
        "   \e[1m%%d\e[0m prints the argument as an int.\n"
        "   \e[1m%%x\e[0m prints the argument as an hexadecimal.\n"
        "   \e[1m%%o\e[0m prints the argument as an octal.\n"
        "   \e[1m%%c\e[0m prints the argument (treated as int) as a char.\n"
        "   \e[1m%%f\e[0m prints the argument as a float.\n"
        "   \e[1m%%<number>X\e[0m prints <number> padding for N (N being one of the previous formats).\n   if <number> is negative, do left instead of right padding.\n"
        "\e[2;3m%s is part of the cutie project hosted under https://github.com/usr-undeleted/cutie licensed under the GPLv3 license.\e[0m\n",
        invocation, invocation, invocation, invocation
    );
    exit(0);
}


void print(char *str, int argc, char *argv[], int *argIdx) {
    for (char *p = str; *p; p++) {

        // escape chars
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
                case 'b': putchar('\b'); break;
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
        } else if (*p == '%' && *(p+1)) {
            p++;
            // used incase user formatting is invalid, we fill it later
            char formatting[32] = { 0 };
            unsigned int check = 1;
            unsigned int fIdx = 1;
            formatting[0] = '%';
            formatting[fIdx++] = *p;

            // padding calculation
            unsigned int leftPad = 0;
            if (*p == '-') { leftPad = 1; p++; };

            char numBuf[32] = { 0 };
            unsigned int i = 0;

            while (isdigit(*p)) {
                numBuf[i++] = *p;
                formatting[fIdx++] = *p;
                p++;
            }
            int width = atoi(numBuf);

            // types
            switch (*p) {
                case '%': putchar('%'); break;
                // strings
                case 's':
                    if (*argIdx < argc) {
                        width -= strlen(argv[(*argIdx)]);
                        if (leftPad) {
                            for (int i = 0; i < width; i++) putchar(' ');
                        }
                        printf("%s", argv[(*argIdx)++]);

                        if (width && !leftPad) {
                            for (int i = 0; i < width; i++) putchar(' ');
                        }

                    } else {
                        printf("Not enough arguments provided for formatting. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
                        exit(2);
                    }
                    break;
                // numbers
                case 'd':
                    if (*argIdx < argc) {
                        width -= strlen(argv[(*argIdx)]);
                        if (leftPad) {
                            for (int i = 0; i < width; i++) putchar(' ');
                        }
                        printf("%d", atoi(argv[(*argIdx)++]));

                        if (width && !leftPad) {
                            for (int i = 0; i < width; i++) putchar(' ');
                        }

                    } else {
                        printf("Not enough arguments provided for formatting. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
                        exit(2);
                    }
                    break;

                // floats
                case 'f':
                if (*argIdx < argc) {
                    width -= strlen(argv[(*argIdx)]);
                    if (leftPad) {
                        for (int i = 0; i < width; i++) putchar(' ');
                    }
                    printf("%*f", width, atof(argv[(*argIdx)++]));

                    if (width && !leftPad) {
                        for (int i = 0; i < width; i++) putchar(' ');
                    }

                } else {
                    printf("Not enough arguments provided for formatting. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
                    exit(2);
                }
                break;

                // hexa
                case 'x':
                if (*argIdx < argc) {
                    width -= strlen(argv[(*argIdx)]);
                    if (leftPad) {
                        for (int i = 0; i < width; i++) putchar(' ');
                    }
                    printf("%x", atoi(argv[(*argIdx)++]));

                    if (width && !leftPad) {
                        for (int i = 0; i < width; i++) putchar(' ');
                    }

                } else {
                    printf("Not enough arguments provided for formatting. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
                    exit(2);
                }
                break;

                // octal
                case 'o':
                if (*argIdx < argc) {
                    width -= strlen(argv[(*argIdx)]);
                    if (leftPad) {
                        for (int i = 0; i < width; i++) putchar(' ');
                    }
                    printf("%o", atoi(argv[(*argIdx)++]));

                    if (width && !leftPad) {
                        for (int i = 0; i < width; i++) putchar(' ');
                    }

                } else {
                    printf("Not enough arguments provided for formatting. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
                    exit(2);
                }
                break;

                // chars
                case 'c':
                if (*argIdx < argc) {
                    width--;
                    if (leftPad) {
                        for (int i = 0; i < width; i++) putchar(' ');
                    }
                    printf("%c", atoi(argv[(*argIdx)++]));

                    if (width && !leftPad) {
                        for (int i = 0; i < width; i++) putchar(' ');
                    }

                } else {
                    printf("Not enough arguments provided for formatting. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
                    exit(2);
                }
                break;

                default:
                    formatting[fIdx] = *p;
                    printf("Unknown formatting '%s'. See '%s -h' or '%s --help' for instructions.\n", formatting, argv[0], argv[0]);
                    exit(2);
                    break;
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

    // manage flags
    // we dont use labelflags here cus like, why
    // malloc for a single flag???
    for (int i = 0; i < argc; i++) {
        if (argv[i][0] != '-') continue;

        if (!strcmp(argv[i], "--help")) {
            helpMenu(argv[0]);
        } else {
            fprintf(stderr, "Invalid flag detected. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
            return 2;
        }

        for (int j = 0; j < strlen(argv[i]); j++) {
            if (argv[i][j] == 'h') {
                helpMenu(argv[0]);
            } else {
                fprintf(stderr, "Invalid flag detected. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
                return 2;
            }
        }
    }

    // after invocation and format
    int argIdx = 2;

    char *format;
    char stdbuf[4096];
    if (piped && (!strcmp(argv[1], "-") || !strcmp(argv[1], "--"))) {
        size_t n = fread(stdbuf, 1, sizeof(stdbuf) - 1, stdin);
        stdbuf[n] = '\0';
        format = stdbuf;
    } else {
        format = argv[1];
    }
    print(format, argc, argv, &argIdx);

    return 0;
}
