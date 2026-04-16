#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>

// find contents in dir, allocate string with entries
// int listFiles (char *dir) {}

void mallocErr() {
    printf("Malloc has failed to run. Aborting command.\n");
    exit(1);
}

int main (int argc, char *argv[]) {
    if (argc == 1) {
        char *buf;
        long size = pathconf(".", _PC_PATH_MAX);

        if ((buf = (char *)malloc((size_t)size)) != NULL) {
            char *pwd = getcwd(buf, (size_t)size);
            printf("%s\n", pwd);
        } else {
            mallocErr();
        }
        // listFiles(pwd);
    }
    return 0;
}
