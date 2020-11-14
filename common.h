#ifndef _COMMON_H_
#define _COMMON_H_

#define DEBUGGING

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define FALSE   0
#define TRUE    1

#define log(...) do { \
    fprintf(stderr, "[*] "); \
    fprintf(stderr, __VA_ARGS__); \
} while (0)

#ifdef DEBUGGING

#include <stdio.h>

#define debug_print(...) do {                                   \
    FILE *fp = fopen("/tmp/gssh.log", "a");                     \
    fprintf(fp, "[%d:%s:%d] ", getpid(), __FILE__, __LINE__);   \
    fprintf(fp, __VA_ARGS__);                                   \
    fclose(fp);                                                 \
} while (0)

// #define debug_print(...) do {                                   \
//     fprintf(stderr, "[%d:%s:%d] ", getpid(), __FILE__, __LINE__);   \
//     fprintf(stderr, __VA_ARGS__);                                   \
// } while (0)

#else // DEBUGGING

#define debug_print(...) ;

#endif // DEBUGGING

#endif // _COMMON_H_
