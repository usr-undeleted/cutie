#include <openssl/sha.h>
#include <stdio.h>
#include "cutie-common.h"
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

// experimental; split hash processing beetwen multiple threads.
// not reliable unless youre hashing something like /bin/* files, as there, ive found
// that multithreading halves the time needed to hash, even at 512 mode!
// compile with -DUSE_OMP -fopenmp, requires openmp (arch linux, for example, has that pkg)
// pair with USE_SORTED to make output be actually organized
#ifdef USE_OMP
    #include <omp.h>
#endif

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
    #if defined(SHA256D)
        printf(
            "   \e[1m%s\e[0m <flags> <files>\n"
            "   \e[1m%s\e[0m will find the hash for the algorithm SHA-256\n",
            invocation, invocation
        );

    #elif defined(SHA512D)
        printf(
            "   \e[1m%s\e[0m <flags> <files>\n"
            "   \e[1m%s\e[0m will find the hash for the algorithm SHA-512\n",
            invocation, invocation
        );

    #elif defined(SHA1D)
        printf(
            "   \e[1m%s\e[0m <flags> <files>\n"
            "   \e[1m%s\e[0m will find the hash for the algorithm SHA-1\n",
            invocation, invocation
        );

    #elif defined(SHA224D)
        printf(
            "   \e[1m%s\e[0m <flags> <files>\n"
            "   \e[1m%s\e[0m will find the hash for the algorithm SHA-224\n",
            invocation, invocation
        );

    #elif defined(SHA384D)
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
        "\e[1m%s\e[0m will make the hash for all files specified. If stdin is provided and you specify '-' as one of the arguments, print stdin's hash. Stdin can be used more than once without providing '\\0' hash after the first hash.\n"
        "note that the order of flags and files don't matter.\n\n"
        "flags:\n"
        "   \e[1m-h\e[0m or \e[1m--help\e[0m: show this menu.\n\n"
        "\e[2;3m%s is part of the cutie project hosted under https://github.com/usr-undeleted/cutie licensed under the GPLv3 license.\e[0m\n",
        invocation, invocation
    );
    exit(0);
}

// only use argv[2+] (or 1+ if compiled for specific algo) for flags and files
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
    // start idx is used later, not related to flags
    int startIdx;
    #if defined(SHA256D) || defined(SHA512D) || defined(SHA1D) || defined(SHA224D) || defined(SHA384D)
        flags = labelFlags(argc - 1, argv + 1, &input);
        startIdx = 1;
    #else
        flags = labelFlags(argc - 2, argv + 2, &input);
        startIdx = 2;
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
    #if defined(SHA256D)
        algo.digestLen = SHA256_DIGEST_LENGTH;
        algo.init = (void(*)(void*))SHA256_Init;
        algo.update = (void(*)(void*, const void*, size_t))SHA256_Update;
        algo.final = (void(*)(unsigned char*, void*))SHA256_Final;
        algo.ctxSize = sizeof(SHA256_CTX);

    #elif defined(SHA512D)
        algo.digestLen = SHA512_DIGEST_LENGTH;
        algo.init = (void(*)(void*))SHA512_Init;
        algo.update = (void(*)(void*, const void*, size_t))SHA512_Update;
        algo.final = (void(*)(unsigned char*, void*))SHA512_Final;
        algo.ctxSize = sizeof(SHA512_CTX);

    #elif defined(SHA1D)
        algo.digestLen = SHA_DIGEST_LENGTH;
        algo.init = (void(*)(void*))SHA1_Init;
        algo.update = (void(*)(void*, const void*, size_t))SHA1_Update;
        algo.final = (void(*)(unsigned char*, void*))SHA1_Final;
        algo.ctxSize = sizeof(SHA_CTX);

    #elif defined(SHA224D)
        algo.digestLen = SHA224_DIGEST_LENGTH;
        algo.init = (void(*)(void*))SHA224_Init;
        algo.update = (void(*)(void*, const void*, size_t))SHA224_Update;
        algo.final = (void(*)(unsigned char*, void*))SHA224_Final;
        algo.ctxSize = sizeof(SHA256_CTX);

    #elif defined(SHA384D)
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

    unsigned int noFiles = 1;
    unsigned int useStdin = 0;
    unsigned int stdinHashed = 0; // in omp mode, use this to indicate wether or not to digest stdin
    unsigned char stdinDigest[64];
    if (!isatty(STDIN_FILENO)) {
        useStdin = 1;

    }

    // collect files
    #ifdef USE_OMP
    int fileIndices[argc];
    int fileCount = 0;
    for (int i = startIdx; i < argc; i++) {
        if (argv[i][0] == '-' && !useStdin) continue;
        // handle stdin separately
        if (!strcmp(argv[i], "-") && useStdin) {
            stdinHashed = 1;
            continue;
        }

        fileIndices[fileCount++] = i;
    }
    // digest stdin; will only be used once, sadly
    if (stdinHashed) {
        void *ctx = malloc(algo.ctxSize);
        FILE *file = fdopen(STDIN_FILENO, "r");
        char buf[4096];
        size_t n;
        algo.init(ctx);

        while ((n = fread(buf, 1, sizeof(buf), file)) > 0) {
            algo.update(ctx, buf, n);
        }

        algo.final(stdinDigest, ctx);
        free(ctx);
    }

    struct result { unsigned char digest[64]; int error; };
    struct result *results = calloc(fileCount, sizeof(*results));

    #endif

    #ifdef USE_OMP
    #pragma omp parallel for reduction(|:errorCode)
    for (int i = 0; i < fileCount; i++) {
        int idx = fileIndices[i];
        int localError = 0;
    #else
    for (int i = startIdx; i < argc; i++) {
    #endif
        #ifndef USE_OMP
        if (argv[i][0] == '-' && !useStdin) continue;
        int isStdin = 0;
        if (!strcmp(argv[i], "-") && useStdin) isStdin = 1;
        #endif
        noFiles = 0;

        void *ctx = malloc(algo.ctxSize);
        if (!ctx) {
            perror("Failed to allocate memory");
            #ifndef USE_OMP
            return 1;
            #endif
        }

        #ifndef USE_OMP
        int fd = isStdin ? STDIN_FILENO : open(argv[i], O_RDONLY);
        if (!isStdin) {
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
        }
        #else
        // open file
        int fd = open(argv[idx], O_RDONLY);
        if (fd == -1) {
            fprintf(stderr, "Failed to read file '%s': %s\n", argv[idx], strerror(errno));
            results[i].error = 2;
            localError = 1;
        }
        // is dir?
        if (!localError) {
            struct stat st;
            fstat(fd, &st);
            if (S_ISDIR(st.st_mode)) {
                fprintf(stderr, "Failed to hash file '%s': Is a directory\n", argv[idx]);
                close(fd);
                results[i].error = 2;
                localError = 1;
            }
        }
        #endif

        #ifndef USE_OMP
        FILE *file;
        file = fdopen(fd, "r");
        if (ferror(file)) {
            fprintf(stderr, "Error reading '%s'\n", argv[i]);
            #ifdef USE_OMP
            results[i].error = 1;
            #endif

            errorCode = 1;
            continue;
        }
        #else
        FILE *file;
        if (!localError) {
            file = fdopen(fd, "r");
            if (ferror(file)) {
                fprintf(stderr, "Error reading '%s'\n", argv[idx]);
                results[i].error = 1;

                localError = 1;
            }
        }
        #endif

        #ifdef USE_OMP
        if (!localError) {
        #endif
        char buf[4096];
        size_t n;
        algo.init(ctx);

        while ((n = fread(buf, 1, sizeof(buf), file)) > 0) {
            algo.update(ctx, buf, n);
        }
        #ifdef USE_OMP
        }
        #endif
        unsigned char digest[64];

        #ifndef USE_OMP
        if (isStdin) {
            if (!stdinHashed) {
                algo.final(stdinDigest, ctx);
                for (int j = 0; j < algo.digestLen; j++) {
                    printf("%02x", stdinDigest[j]);
                }
                stdinHashed = 1;
            } else {
                for (int j = 0; j < algo.digestLen; j++) {
                    printf("%02x", stdinDigest[j]);
                }
            }

        } else {
            algo.final(digest, ctx);
            for (int j = 0; j < algo.digestLen; j++) {
                printf("%02x", digest[j]);
            }
        }

        showFile ? (isStdin ? printf("  {stdin}\n") : printf("  %s\n", argv[i])) : printf("\n");
        #else
        #ifdef USE_SORTED
        algo.final(results[i].digest, ctx);
        #endif
        if (!localError) {
            algo.final(results[i].digest, ctx);
            for (int j = 0; j < algo.digestLen; j++) {
                printf("%02x", results[i].digest[j]);
            }
            printf("  %s\n", argv[idx]);
        }
        #endif
        #ifndef USE_OMP
        if (!isStdin) fclose(file);
        #else
        if (!localError) fclose(file);
        #endif
        free(ctx);
    }
    #ifdef USE_SORTED
    for (int l = 0; l < fileCount; l++) {
        if (results[l].error) continue;
        for (int j = 0; j < algo.digestLen; j++) {
            printf("%02x", results[l].digest[j]);
        }
        printf("  %s\n", argv[fileIndices[l]]);
    }
    free(results);
    #endif

    #ifdef USE_OMP
    if (stdinHashed) {
        for (int k = 0; k < algo.digestLen; k++) {
            printf("%02x", stdinDigest[k]);
        }
        printf("  {stdin}\n");
    }
    #endif

    if (noFiles) {
        fprintf(stderr, "No files provided. See '%s -h' or '%s --help' for instructions.\n", argv[0], argv[0]);
        return 2;
    }

    return errorCode;
}
