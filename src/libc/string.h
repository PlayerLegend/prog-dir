#ifndef FLAT_INCLUDES
#include <sys/types.h>
#define FLAT_INCLUDES
#endif

void *memcpy(void *dest, const void *src, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
size_t strlen(const char *s);
