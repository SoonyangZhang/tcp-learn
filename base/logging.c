#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include "logging.h"

static const char *LEVEL_MAP[] = {"DEBUG", "INFO", "WARN", "ERROR", "CRIT"};


void logger(const char *file, int line,const char *function, int level, const char *fmt, ...)
{
    va_list ap;
    char msg[MAX_LOG_LEN];
    char now[256] = {"my log"};
    pid_t process_id = getpid();

    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    fprintf(stderr, "%s %s,%d,%s,%s\n", LEVEL_MAP[level],file,line,function,msg);


}
