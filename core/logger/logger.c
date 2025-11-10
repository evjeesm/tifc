#include "logger.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

logger_status_t logger_init(logger_t *const logger,
        const severity_t severity,
        log_path_t path)
{
    FILE *output = fopen(path, "a");
    if (!output) {
        perror("logger");
        return LOGGER_FAIL;
    }

    *logger = (logger_t){
        .severity = severity,
        .output = output,
    };

    return LOGGER_SUCCESS;
}


logger_status_t logger_log(logger_t *restrict const logger,
        const severity_t severity,
        const char *restrict format,
        ...)
{
    if (severity < logger->severity) return LOGGER_SKIPPED;

    va_list list;
    va_start(list, format);
#ifdef ESCAPE_COLORS
#ifndef ESC
#   define ESC "\x1b"
#endif

    static const char *const EscapeColors[] = {
        [LOGGER_INFO]     = ESC "[0m",
        [LOGGER_DEBUG]    = ESC "[36m",
        [LOGGER_WARNING]  = ESC "[33m",
        [LOGGER_CRITICAL] = ESC "[91m"
    };
    fprintf(logger->output, "%s", EscapeColors[severity]);
#endif
    int bytes = vfprintf(logger->output, format, list);
    if (bytes < 0)
    {
        perror("logger");
        return LOGGER_FAIL;
    }
#ifdef ESCAPE_COLORS
    fprintf(logger->output, "%s", EscapeColors[0]);
#endif
    fflush(logger->output);
    va_end(list);
    return LOGGER_SUCCESS;
}

void logger_deinit(logger_t *const logger)
{
    fclose(logger->output);
}

static void logger_static_cleanup(void);
logger_t *logger_static(void)
{
    static logger_t *Instance = 0;
    static logger_t Logger;

    if (Instance) return Instance;

    LOG_INIT(&Logger, S_LOG_SEVERITY, S_LOG_PATH);

    atexit(logger_static_cleanup);

    Instance = &Logger;
    return Instance;
}

static void logger_static_cleanup(void)
{
    logger_deinit(logger_static());
}

