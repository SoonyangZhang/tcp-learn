#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <malloc.h>
#include "logging.h"
#include "alloc.h"

void *cv_raw_malloc(size_t size, const char *file, int line)
{
    void *ptr = malloc(size);
    if (ptr == NULL) {
        LOG(ERROR, "Fetal: OOM trying to allocate %d bytes at %s:%d", size,
                file, line);
        abort();
    }
    return ptr;
}

void *cv_raw_calloc(size_t number, size_t size, const char *file, int line)
{
    void *ptr = calloc(number, size);
    if (ptr == NULL) {
        LOG(ERROR, "Fetal: OOM trying to allocate %d bytes at %s:%d",
                number * size, file, line);
        abort();
    }
    return ptr;
}

void *cv_raw_realloc(void *ptr, size_t size, const char *file, int line)
{
    void *newptr =realloc(ptr, size);
    if (newptr == NULL) {
        LOG(ERROR, "Fetal: OOM trying to allocate %d bytes at %s:%d", size,
                file, line);
        abort();
    }
    return newptr;
}

void cv_raw_free(void *ptr)
{
    free(ptr);
}

char *cv_raw_strndup(const char *other, size_t size, const char *file, int line)
{
    char *p = cv_raw_malloc(size + 1, file, line);
    strncpy(p, other, size);
    p[size] = '\0';
    return p;
}
