#ifndef LOGGER_H__
#define LOGGER_H__

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

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

void LoggerInstruction(
    LOGGING_LEVEL loggingLevel,
    uint32_t opcode,
    uint32_t funct3,
    uint32_t funct7,
    uint32_t rd,
    uint32_t rs1,
    uint32_t rs2,
    uint32_t imm
);

#endif // LOGGER_H__