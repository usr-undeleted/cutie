#include "cutie-common.h"
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

pid_t childPid;
int killSig = SIGTERM;
struct { char *name; int sig; } const sigMap[] = {
    {"HUP",     SIGHUP},
    {"INT",     SIGINT},
    {"QUIT",    SIGQUIT},
    {"ILL",     SIGILL},
    {"TRAP",    SIGTRAP},
    {"ABRT",    SIGABRT},
    {"BUS",     SIGBUS},
    {"FPE",     SIGFPE},
    {"KILL",    SIGKILL},
    {"USR1",    SIGUSR1},
    {"SEGV",    SIGSEGV},
    {"USR2",    SIGUSR2},
    {"PIPE",    SIGPIPE},
    {"ALRM",    SIGALRM},
    {"TERM",    SIGTERM},
    {"STKFLT",  SIGSTKFLT},
    {"CHLD",    SIGCHLD},
    {"CONT",    SIGCONT},
    {"STOP",    SIGSTOP},
    {"TSTP",    SIGTSTP},
    {"TTIN",    SIGTTIN},
    {"TTOU",    SIGTTOU},
    {"URG",     SIGURG},
    {"XCPU",    SIGXCPU},
    {"XFSZ",    SIGXFSZ},
    {"VTALRM",  SIGVTALRM},
    {"PROF",    SIGPROF},
    {"WINCH",   SIGWINCH},
    {"IO",      SIGIO},
    {"PWR",     SIGPWR},
    {"SYS",  SIGSYS},
};

void exterminateChild(int sig) {
    kill(childPid, killSig);
}

void helpMenu(char *invocation) {
    printf("\e[1m%s\e[0m command basic usage:\n"
        "   \e[1m%s\e[0m <flags> <timeout> <command>\e[0m\n"
        "   flags HAVE to be before the timeout. %s will consider anything after the flags as a timeout, then the command.\n"
        "   the command doesnt need to be a single string.\n\n"
        "flags:\n"
        "   \e[1m-h\e[0m or \e[1m--help\e[0m: show this menu.\n"
        "   \e[1m-s\e[0m or \e[1m--signal\e[0m: specify signal to be sent to child proc.\n\n"
        "\e[2;3m%s is part of the cutie project hosted under https://github.com/usr-undeleted/cutie licensed under the GPLv3 license.\e[0m\n",
        invocation, invocation, invocation, invocation
    );
    exit(0);
}

int main (int argc, char *argv[]) {
    // manage flags
    // we dont use labelflags here cus like, why
    // malloc for a single flag??? + we need to
    // handle -s and its argument
    int timeoutPos = 0;
    int timeoutDuration = 0;
    int invalidFlag = 0;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            timeoutPos = i;
            timeoutDuration = atoi(argv[timeoutPos]);
            break;
        };

        if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            helpMenu(argv[0]);
        } else {
            invalidFlag = 1;
        }

        if (!strcmp(argv[i], "--signal") || !strcmp(argv[i], "-s")) {
            invalidFlag = 0;
            if (i + 1 > argc) {
                fprintf(stderr, "Not enough arguments given. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
                return 2;
            }

            unsigned int matched = 0;
            killSig = atoi(argv[i + 1]);
            // atoi hands 0 at something like SIGINT, so we manually check from sigMap[]
            if (!killSig) {
                for (int j = 0; j < 31; j++) {
                    int offset = 0;
                    if (!strncmp(argv[i + 1], "SIG", 3)) offset = 3;

                    if (!strcmp(argv[i + 1] + offset, sigMap[j].name)) {
                        matched = 1;
                        killSig = j;
                        break;
                    }
                }
            }
            // check if its valid
            if (killSig > 0 && killSig < 32) matched = 1;

            if (!matched) {
                fprintf(stderr, "Invalid signal given. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
                return 2;
            }
            i++;
            continue;
        }  else {
            invalidFlag = 1;
        }

        if (invalidFlag) {
            fprintf(stderr, "Invalid flag detected. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
            return 2;
        }
    }

    if (!timeoutPos) {
        fprintf(stderr, "No timeout specified. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
        return 2;
    }

    if (argc <= (timeoutPos + 1)) {
        fprintf(stderr, "No command specified. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
        return 2;
    }

    if (!timeoutDuration) {
        fprintf(stderr, "Timeout is invalid. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
        return 2;
    }

    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Couldn't fork program.\n");
        return 1;
    }
    childPid = pid;
    signal(SIGALRM, exterminateChild);

    if (pid == 0) {
        // child
        int ret = 1;
        ret = execvp(argv[(timeoutPos + 1)], &argv[(timeoutPos + 1)]);
        if (ret == -1) {
            fprintf(stderr, "Couldn't execute command: %s\n", strerror(errno));
            return 2;
        }
    } else {
        // parent
        alarm(timeoutDuration);
        int status;
        waitpid(pid, &status, 0);
        alarm(0);
    }

    return 0;
}
