#ifndef LOGGER_H__
#define LOGGER_H__

#include <stdio.h>
#include <stdarg.h>

typedef enum LOGGING_LEVEL
{
    DEBUG,
    SUCCESS,
    INFO,
    WARNING,
    ERROR,
    CRITICAL,
} LOGGING_LEVEL;

void LoggerInitialize(
    LOGGING_LEVEL minimumLevel,
    FILE *stream
);

void LoggerMessage(
    LOGGING_LEVEL loggingLevel, 
    const char *message, 
    ...
);

void LoggerSetOuput(
    FILE *stream
);

#endif // LOGGER_H__