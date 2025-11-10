#include "logger.h"

int main(void)
{
    logger_t Logger;
    logger_init(&Logger, LOGGER_INFO, "./my_log.log");

    LOG(&Logger, LOGGER_INFO,
        "Hello from logger: %d %#x %s \n", 255, 255, "string");

    LOG(&Logger, LOGGER_CRITICAL,
        "ERROR:: from logger: %d %#x %s \n", 255, 255, "string");

    logger_deinit(&Logger);
    return 0;
}


