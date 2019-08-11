#ifndef ALLOC_H
#define ALLOC_H
#ifdef __cplusplus
extern "C" {
#endif

#define cv_malloc(size) cv_raw_malloc(size, __FILE__, __LINE__)
#define cv_calloc(number, size) cv_raw_calloc(number, size, __FILE__, __LINE__)
#define cv_realloc(ptr, size) cv_raw_realloc(ptr, size, __FILE__, __LINE__)
#define cv_strndup(other, size) cv_raw_strndup(other, size, __FILE__, __LINE__)
#define cv_free(ptr) cv_raw_free(ptr)
void *cv_raw_malloc(size_t size, const char *file, int line);
void *cv_raw_calloc(size_t number, size_t size, const char *file, int line);
void *cv_raw_realloc(void *ptr, size_t size, const char *file, int line);
void cv_raw_free(void *ptr);
char *cv_raw_strndup(const char *other, size_t size, const char *file, int line);
#ifdef __cplusplus
}
#endif
#endif /* end of include guard: ALLOC_H */
