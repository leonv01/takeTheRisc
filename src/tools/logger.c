#include "logger.h"
#include <stdio.h>

typedef struct Logger
{
    FILE *stream;

    LOGGING_LEVEL minLevel;
} Logger;

static Logger logger = { 0 };

/* ---------------------------- Helper functions ---------------------------- */

static const char *LevelToString(LOGGING_LEVEL type)
{
    switch (type)
    {
        case INFO:      return "[INFO] ";
        case WARNING:   return "[WARNING] ";
        case ERROR:     return "[ERROR] ";
        case CRITICAL:  return "[CRITICAL] ";
        case SUCCESS:   return "[SUCCESS] ";
        case DEBUG:     return "[DEBUG] ";
        default:        return "[LOG] ";
    }
}

/* -------------------------------------------------------------------------- */

void LoggerInitialize(
    LOGGING_LEVEL minimumLevel,
    FILE *stream
)
{
    logger.stream = stream;
    logger.minLevel = minimumLevel;
}

void LoggerMessage(
    LOGGING_LEVEL loggingLevel, 
    const char *message, 
    ...
)
{
    if (loggingLevel < logger.minLevel) return;

    char buffer[1024];
    va_list args;
    va_start(args, message);
    vsnprintf(buffer, sizeof(buffer), message, args);
    va_end(args);

    if (logger.stream)
    {
        fprintf(logger.stream, "%s %s\n", LevelToString(loggingLevel), buffer);
        fflush(logger.stream);
    }
}

void LoggerSetOuput(FILE *stream)
{
    logger.stream = stream;
}
