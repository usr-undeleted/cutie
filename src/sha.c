#include <openssl/sha.h>
#include <stdio.h>
#include "cutie-common.h"
#include <fcntl.h>
#include <errno.h>

unsigned int showFile = 1;

struct algo {
    int digestLen;
    void (*init)(void *ctx);
    void (*update)(void *ctx, const void *data, size_t len);
    void (*final)(unsigned char *md, void *ctx);
    size_t ctxSize;
};

void helpMenu(char *invocation) {
    printf("\e[1m%s\e[0m command basic usage:\n", invocation);
    #if defined(SHA256)
        printf(
            "   \e[1m%s\e[0m <flags> <files>\n"
            "   \e[1m%s\e[0m will find the hash for the algorithm SHA-256\n",
            invocation, invocation
        );

    #elif defined(SHA512)
        printf(
            "   \e[1m%s\e[0m <flags> <files>\n"
            "   \e[1m%s\e[0m will find the hash for the algorithm SHA-512\n",
            invocation, invocation
        );

    #elif defined(SHA1)
        printf(
            "   \e[1m%s\e[0m <flags> <files>\n"
            "   \e[1m%s\e[0m will find the hash for the algorithm SHA-1\n",
            invocation, invocation
        );

    #elif defined(SHA224)
        printf(
            "   \e[1m%s\e[0m <flags> <files>\n"
            "   \e[1m%s\e[0m will find the hash for the algorithm SHA-224\n",
            invocation, invocation
        );

    #elif defined(SHA224)
        printf(
            "   \e[1m%s\e[0m <flags> <files>\n"
            "   \e[1m%s\e[0m will find the hash for the algorithm SHA-384\n",
            invocation, invocation
        );

    #else
        printf(
            "   \e[1m%s\e[0m <algorithm> <flags> <files>\n"
            "   \e[1m%s\e[0m algorithms: 1, 224, 256, 384, 512\n",
            invocation, invocation
        );
    #endif

    printf(
        "\e[1m%s\e[0m will make the hash for all files specified.\n"
        "\e[1m%s\e[0m note that the order of flags and files don't matter.\n\n"
        "flags:\n"
        "   \e[1m-h\e[0m or \e[1m--help\e[0m: show this menu.\n\n"
        "\e[2;3m%s is part of the cutie project hosted under https://github.com/usr-undeleted/cutie licensed under the GPLv3 license.\e[0m\n",
        invocation, invocation, invocation
    );
    exit(0);
}

// only use argv[2+] for flags and files
int main (int argc, char *argv[]) {
    if (argc == 1) {
        fprintf(stderr, "No algorithm provided. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
        return 2;
    }
    if (argc == 2) {
        fprintf(stderr, "No files provided. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
        return 2;
    }
    int errorCode = 0;

    // manage flags
    char charFlags[] = {
        'h',
        'f'
    };
    char *stringFlags[] = {
        "--help",
        "--hide-file"
    };
    int charLen = sizeof(charFlags) / sizeof(charFlags[0]);
    int stringLen = sizeof(stringFlags) / sizeof(stringFlags[0]);
    struct flagInput input = {
        charFlags, charLen,
        stringFlags, stringLen,
        0
    };

    int *flags;
    #if defined(SHA256) || defined(SHA512) || defined(SHA1) || defined(SHA224) || defined(SHA384)
        flags = labelFlags(argc - 1, argv + 1, &input);
    #else
        flags = labelFlags(argc - 2, argv + 2, &input);
    #endif

    if (flags != NULL) {
        for (int i = 0; i < input.flagCount; i++) {
            if (flags[i] == 0) helpMenu(argv[0]);

            if (flags[i] == 1) showFile = 0;
        }
    } else {
        fprintf(stderr, "Invalid flag detected. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
        return 2;
    }
    free(flags);

    // find what algorithm the user wants (at the bottom)
    // define these to make the binary smaller by packing
    // a single algorithm
    struct algo algo;
    #if defined(SHA256)
        algo.digestLen = SHA256_DIGEST_LENGTH;
        algo.init = (void(*)(void*))SHA256_Init;
        algo.update = (void(*)(void*, const void*, size_t))SHA256_Update;
        algo.final = (void(*)(unsigned char*, void*))SHA256_Final;
        algo.ctxSize = sizeof(SHA256_CTX);

    #elif defined(SHA512)
        algo.digestLen = SHA512_DIGEST_LENGTH;
        algo.init = (void(*)(void*))SHA512_Init;
        algo.update = (void(*)(void*, const void*, size_t))SHA512_Update;
        algo.final = (void(*)(unsigned char*, void*))SHA512_Final;
        algo.ctxSize = sizeof(SHA512_CTX);

    #elif defined(SHA1)
        algo.digestLen = SHA_DIGEST_LENGTH;
        algo.init = (void(*)(void*))SHA1_Init;
        algo.update = (void(*)(void*, const void*, size_t))SHA1_Update;
        algo.final = (void(*)(unsigned char*, void*))SHA1_Final;
        algo.ctxSize = sizeof(SHA_CTX);

    #elif defined(SHA224)
        algo.digestLen = SHA224_DIGEST_LENGTH;
        algo.init = (void(*)(void*))SHA224_Init;
        algo.update = (void(*)(void*, const void*, size_t))SHA224_Update;
        algo.final = (void(*)(unsigned char*, void*))SHA224_Final;
        algo.ctxSize = sizeof(SHA256_CTX);

    #elif defined(SHA384)
        algo.digestLen = SHA384_DIGEST_LENGTH;
        algo.init = (void(*)(void*))SHA384_Init;
        algo.update = (void(*)(void*, const void*, size_t))SHA384_Update;
        algo.final = (void(*)(unsigned char*, void*))SHA384_Final;
        algo.ctxSize = sizeof(SHA512_CTX);

    #else
        // this is the actual code
        // the definition code blocks of doom
        if (!strcmp(argv[1], "256")) {
            algo.digestLen = SHA256_DIGEST_LENGTH;
            algo.init = (void(*)(void*))SHA256_Init;
            algo.update = (void(*)(void*, const void*, size_t))SHA256_Update;
            algo.final = (void(*)(unsigned char*, void*))SHA256_Final;
            algo.ctxSize = sizeof(SHA256_CTX);

        } else if (!strcmp(argv[1], "512")) {
            algo.digestLen = SHA512_DIGEST_LENGTH;
            algo.init = (void(*)(void*))SHA512_Init;
            algo.update = (void(*)(void*, const void*, size_t))SHA512_Update;
            algo.final = (void(*)(unsigned char*, void*))SHA512_Final;
            algo.ctxSize = sizeof(SHA512_CTX);

        } else if (!strcmp(argv[1], "1")) {
            algo.digestLen = SHA_DIGEST_LENGTH;
            algo.init = (void(*)(void*))SHA1_Init;
            algo.update = (void(*)(void*, const void*, size_t))SHA1_Update;
            algo.final = (void(*)(unsigned char*, void*))SHA1_Final;
            algo.ctxSize = sizeof(SHA_CTX);

        } else if (!strcmp(argv[1], "224")) {
            algo.digestLen = SHA224_DIGEST_LENGTH;
            algo.init = (void(*)(void*))SHA224_Init;
            algo.update = (void(*)(void*, const void*, size_t))SHA224_Update;
            algo.final = (void(*)(unsigned char*, void*))SHA224_Final;
            algo.ctxSize = sizeof(SHA256_CTX);

        } else if (!strcmp(argv[1], "384")) {
            algo.digestLen = SHA384_DIGEST_LENGTH;
            algo.init = (void(*)(void*))SHA384_Init;
            algo.update = (void(*)(void*, const void*, size_t))SHA384_Update;
            algo.final = (void(*)(unsigned char*, void*))SHA384_Final;
            algo.ctxSize = sizeof(SHA512_CTX);

        } else {
            fprintf(stderr, "No valid algorithm provided. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
            return 2;
        }
    #endif

    int startIdx;
    #if defined(SHA256) || defined(SHA512) || defined(SHA1) || defined(SHA224) || defined(SHA384)
        startIdx = 1;
    #else
        startIdx = 2;
    #endif

    unsigned int noFiles = 1;
    for (int i = startIdx; i < argc; i++) {
        if (argv[i][0] == '-') continue;
        noFiles = 0;

        void *ctx = malloc(algo.ctxSize);
        if (!ctx) {
            perror("Failed to allocate memory");
            return 1;
        }

        int fd = open(argv[i], O_RDONLY);
        if (fd == -1) {
            fprintf(stderr, "Failed to read file '%s': %s\n", argv[i], strerror(errno));
            if (argc == 1) {
                return 2;
            } else {
                errorCode = 2;
                continue;
            }
        }
        // is dir?
        struct stat st;
        fstat(fd, &st);
        if (S_ISDIR(st.st_mode)) {
            // the fake errno of doom
            fprintf(stderr, "Failed to hash file '%s': Is a directory\n", argv[i]);
            close(fd);
            if (argc == 1) {
                return 2;
            } else {
                errorCode = 2;
                continue;
            }
        }


        FILE *file = fdopen(fd, "r");
        if (ferror(file)) {
            fprintf(stderr, "Error reading '%s'\n", argv[i]);
            errorCode = 1;
            continue;
        }

        char buf[4096];
        size_t n;
        algo.init(ctx);

        while ((n = fread(buf, 1, sizeof(buf), file)) > 0) {
            algo.update(ctx, buf, n);
        }

        unsigned char digest[64];
        algo.final(digest, ctx);
        for (int j = 0; j < algo.digestLen; j++) {
            printf("%02x", digest[j]);
        }

        showFile ? printf("  %s\n", argv[i]) : printf("\n");
        fclose(file);
        free(ctx);
    }

    if (noFiles) {
        fprintf(stderr, "No files provided. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
        return 2;
    }

    return errorCode;
}
