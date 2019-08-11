#ifndef LOGGING_H
#define LOGGING_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>
#define MAX_LOG_LEN 1024

#define LOG(...) logger(__FILE__, __LINE__,__FUNCTION__, __VA_ARGS__)

enum {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    CRIT,
};
void logger(const char *file, int line, const char *function,int level, const char *fmt, ...);
#ifdef __cplusplus
}
#endif
#endif /* end of include guard: LOGGING_H */
