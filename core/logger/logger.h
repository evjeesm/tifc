#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <stdio.h>
#include <assert.h>

#define LOG_INIT(...) do {\
    assert(LOGGER_FAIL != logger_init(__VA_ARGS__)); } while(0)

#define LOG(...) do {\
    assert(LOGGER_FAIL != logger_log(__VA_ARGS__)); } while(0)

#ifndef S_LOG_SEVERITY
#   define S_LOG_SEVERITY LOGGER_DEBUG
#endif
#ifndef S_LOG_PATH
#   define S_LOG_PATH "/tmp/mylog.log"
#endif
#define S_LOG(...) LOG(logger_static(), __VA_ARGS__)

typedef enum {
    LOGGER_INFO = 0,
    LOGGER_DEBUG,
    LOGGER_WARNING,
    LOGGER_CRITICAL
}
severity_t;

typedef struct
{
    severity_t severity;
    FILE *output;
}
logger_t;

typedef const char *const log_path_t;

typedef enum
{
    LOGGER_SUCCESS = 0,
    LOGGER_SKIPPED,
    LOGGER_FAIL,
}
logger_status_t;

logger_status_t logger_init(logger_t *const logger,
        const severity_t severity,
        log_path_t path);

logger_status_t logger_log(logger_t *const logger,
        const severity_t severity,
        const char *format,
        ...);

void logger_deinit(logger_t *const logger);

logger_t *logger_static(void);

#endif//_LOGGER_H_
