#include "cutie-common.h"
#include <pwd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <time.h>

// print new lines beetwen fetches?
unsigned int newLines = 0;
// print a fetch's key?
unsigned int showKey = 0;
// general fetch data, defined by main()
struct utsname unameFetch;
struct sysinfo sysinfoFetch;
unsigned int siValid = 0;
// global errorcode
int errorCode = 0;

char *parseFileValue(const char *path, const char *key) {
    FILE *file = fopen(path, "r");
    if (!file) {
        errorCode = 1;
        return "???";
    };
    static char value[256];

    char line[LINE_MAX];
    size_t keyLen = strlen(key);
    while (fgets(line, sizeof(line), file)) {
        if (!strncmp(line, key, keyLen)) {
            // remove \n
            line[strlen(line) - 1] = '\0';
            // remove key="
            char *start = line + keyLen + 1; // skip equals
            if (*start == '"') start++;

            size_t valLen = strlen(start);
            // remove last "
            if (start[valLen - 1] == '"') start[valLen - 1] = '\0';
            // format everything nicely into returned value
            snprintf(value, sizeof(value), "%s", start);

            fclose(file);
            return value;
        }
    }

    // incase nothing is found
    fclose(file);
    return "???";
}

// table of functions to print info
typedef void (*fetchFunc)(void);
void fetchUser(void)    { printf("%s", getpwuid(getuid())->pw_name ); }
void fetchHost(void)    { printf("%s", unameFetch.nodename ); }
void fetchShell(void)   {
    char *env = getenv("SHELL");
    if (!env) errorCode = 1;
    printf("%s", env ? env : "???");
}
void fetchPname(void)   { printf("%s", parseFileValue("/etc/os-release", "PRETTY_NAME=")); }
void fetchKernel(void)  { printf("%s", unameFetch.release); }
void fetchArch(void)    { printf("%s", unameFetch.machine); }
void fetchCcount(void)  { printf("%ld", sysconf(_SC_NPROCESSORS_ONLN)); }
void fetchUptime(void)  {
    // format total uptime
    long uptime = sysinfoFetch.uptime;

    long weeks = uptime / 604800;
    long days = (uptime % 604800) / 86400;
    long hours = (uptime % 86400) / 3600;
    long mins = (uptime % 3600) / 60;
    long secs = uptime % 60;

    if (weeks)  printf("%ld weeks%c ",      weeks,  days ? ',' : '\0');
    if (days)   printf("%ld days%c ",       days,   hours ? ',' : '\0');
    if (hours)  printf("%ld hours%c ",      hours,  mins ? ',' : '\0');
    if (mins)   printf("%ld minutes%c ",    mins,   secs ? ',' : '\0');
    if (secs)   printf("%ld seconds",       secs);
}
void fetchDate(void) {
    time_t ntime;
    struct tm stime;
    char buf[128];
    time(&ntime);

    stime = *localtime(&ntime);
    // im sorry amerikans!
    strftime(buf, sizeof(buf), "%A %d %B %H:%M:%S %Y", &stime);
    printf("(%ld) %s", ntime, buf);
}

// contains what we will print to fetch
fetchFunc dispath[] = {
    fetchUser,
    fetchHost,
    fetchShell,
    fetchPname,
    fetchKernel,
    fetchArch,
    fetchCcount,
    fetchUptime,
    fetchDate
};

// combine as much user info as realistically possible into
// this single binary
void helpMenu(char *invocation) {
    printf("\e[1m%s\e[0m command basic usage:\n"
        "   \e[1m%s\e[0m <flags> <fetch>\n"
        "\e[1m%s\e[0m will fetch and print the data you specify. note that order of flags and info don't matter.\n\n"
        "flags:\n"
        "   \e[1m--help\e[0m: show this menu.\n"
        "   \e[1m--new-lines\e[0m: print new lines beetwen fetched data.\n"
        "   \e[1m--show-key\e[0m: print fetched data's key before the actual output.\n"
        "   \e[1m--all\e[0m: print all fetch-able data, in the order below.\n\n"
        "fetch-able data:\n"
        "   \e[1m-u\e[0m: display username.\n"
        "   \e[1m-h\e[0m: display hostname.\n"
        "   \e[1m-s\e[0m: display user's shell.\n"
        "   \e[1m-o\e[0m: display OS pretty name.\n"
        "   \e[1m-k\e[0m: display kernel release name.\n"
        "   \e[1m-a\e[0m: display cpu architecture.\n"
        "   \e[1m-c\e[0m: display cpu core count.\n"
        "   \e[1m-t\e[0m: display uptime.\n"
        "   \e[1m-d\e[0m: display current epoch timestamp + date.\n\n"
        "\e[2;3m%s is part of the cutie project hosted under https://github.com/usr-undeleted/cutie licensed under the GPLv3 license.\e[0m\n",
        invocation, invocation, invocation, invocation
    );
    exit(0);
}

#define FETCH_QUANT 9 // single letters
#define FETCH_FF_QUANT 4 // full flags
#define FETCH_KEY_LARGEST 16 // largest key, for padding
int main (int argc, char *argv[]) {
    // eventually loop over this to print
    int dispatchTable[FETCH_QUANT] = { 0 };

    // sadly we cant use labelFlags due to full and minimal flags being different
    char *fullFlags[FETCH_FF_QUANT] = {
        "--help",
        "--new-lines",
        "--show-key",
        "--all"
    };
    char charFlags[FETCH_QUANT] = {
        'u',
        'h',
        's',
        'o',
        'k',
        'a',
        'c',
        't',
        'd'
    };
    char *fetchKey[FETCH_QUANT] = {
        "Username: ",
        "Hostname: ",
        "Shell: ",
        "OS: ",
        "Kernel release: ",
        "Architecture: ",
        "Core count: ",
        "Uptime: ",
        "Date: "
    };

    // set flags and populate dispatch table
    unsigned int fflagMatch = 0;
    unsigned int onlyFail = 1;
    unsigned int invalidFetch = 0;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            fprintf(stderr, "Invalid argument given. See '%s --help' for instructions.\n", argv[0]);
            return 2;
        }

        // full flag mode
        if (argv[i][1] == '-') {
            // cycle trough valid flags and validate
            for (int j = 0; j < FETCH_FF_QUANT; j++) {
                if (!strcmp(argv[i], fullFlags[j])) {
                    fflagMatch = 1;
                    switch (j) {
                        case 0: helpMenu(argv[0]); break;
                        case 1: newLines = 1; break;
                        case 2: showKey = 1; break;
                        case 3:
                        // set every fetch as set
                        onlyFail = 0;
                        for (int k = 0; k < FETCH_QUANT; k++) {
                            dispatchTable[k] = 1;
                        } break;
                    }
                    break;
                } else {
                    fflagMatch = 0;
                }
            }
        }

        if (!fflagMatch) {
            fprintf(stderr, "Invalid flag given. See '%s --help' for instructions.\n", argv[0]);
            return 2;
        } else if (argv[i][1] == '-') continue;
        // data fetch mode
        else {

            // loop trough all items in arg
            for (int j = 1; j < strlen(argv[i]); j++) {

                // loop trough all valid fetches
                for (int k = 0; k < FETCH_QUANT; k++) {
                    if (argv[i][j] == charFlags[k]) {
                        dispatchTable[k] = 1;
                        onlyFail = 0;
                        invalidFetch = 0;
                        break;
                    } else {
                        invalidFetch = 1;
                    }
                }
            }
        }
    }

    if (onlyFail) {
        fprintf(stderr, "No valid fetch given. See '%s --help' for instructions.\n", argv[0]);
        return 2;
    }

    if (invalidFetch) {
        fprintf(stderr, "Invalid fetch given. See '%s --help' for instructions.\n", argv[0]);
        return 2;
    }

    // populate info structs
    uname(&unameFetch);
    if (sysinfo(&sysinfoFetch) == 0) siValid = 1;

    // loop trough dispatch table and display
    for (int i = 0; i < FETCH_QUANT; i++) {
        if (!dispatchTable[i]) continue;
        if (showKey) printf("%*s", FETCH_KEY_LARGEST, fetchKey[i]);
        dispath[i]();
        // space or new lines
        printf("%c", newLines ? '\n' : ' ');
    }

    if (!newLines) putchar('\n');
    return errorCode;
}
