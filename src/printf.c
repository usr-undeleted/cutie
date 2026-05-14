#include "cutie-common.h"

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
        "\e[2;3m%s is part of the cutie project hosted under https://github.com/usr-undeleted/cutie licensed under the GPLv3 license.\e[0m\n",
        invocation, invocation, invocation, invocation
    );
    exit(0);
}


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

        if (!strcmp(argv[i], "--help")) helpMenu(argv[0]);

        for (int j = 0; j < strlen(argv[i]); j++) {
            if (argv[i][j] == 'h') helpMenu(argv[0]);
        }
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
