#include "logger.h"

int main(void)
{
    S_LOG(LOGGER_DEBUG   , "%s\n", "Static Log");

    S_LOG(LOGGER_INFO    , "%s\n",
        "The following messages will be skipped:"
        "   Define S_LOG_SEVERITY and S_LOG_PATH prior"
        "   \"logger.h\" inclusion to change the defaults.");

    S_LOG(LOGGER_CRITICAL, "%s\n", "STATIC LOG will destroy itself upon exit.");
}
