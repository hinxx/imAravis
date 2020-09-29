#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <sys/time.h>

// some handy macros for printing to stderr
#define E(fmt, ...)         do { \
    struct timeval tv; \
    gettimeofday(&tv, NULL); \
    fprintf(stderr, "[%06ld.%06ld] %s:%d ** ERROR ** " fmt, \
        tv.tv_sec, tv.tv_usec, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } while (0)
#ifdef DEBUG
    #define D0(fmt, ...)        do { fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)
    #define D(fmt, ...)         do { \
    struct timeval tv; \
    gettimeofday(&tv, NULL); \
    fprintf(stderr, "[%06ld.%06ld] %s:%d " fmt, \
        tv.tv_sec, tv.tv_usec, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } while (0)
#else
    #define D0(fmt, ...)        do{}while(0)
    #define D(fmt, ...)         do{}while(0)
#endif

#endif // COMMON_H
