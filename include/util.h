#ifndef UTIL_H
#define UTIL_H

#define NEW(TYPE) (TYPE*)malloc(sizeof(TYPE))
#define NEWN(TYPE, N) (TYPE*)malloc(sizeof(TYPE) * (N))
#define eprintf(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)
#define eprintf0(str) fprintf(stderr, "%s", str)

#define TP(...)
#define TEMPLATE(...)

#define COUNTOF(arr) (sizeof(arr) / sizeof((arr)[0]))

#define UNREACHABLE { assert(0 && "unreachable!\n"); }

#endif
