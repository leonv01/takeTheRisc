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

static const char *LogArithmeticR(
    uint32_t funct3,
    uint32_t funct7
)
{
    uint32_t funct10 = funct7 << 3 | funct3;

    switch (funct10)
    {
        case 0b0000000000: return "[Type-R: ADD]";
        case 0b0100000000: return "[Type-R: SUB]";
        case 0b0000000111: return "[Type-R: AND]";
        case 0b0000000110: return "[Type-R: OR]";
        case 0b0000000100: return "[Type-R: XOR]";
        case 0b0000000001: return "[Type-R: SLL]";
        case 0b0000000101: return "[Type-R: SRL]";
        case 0b0100000101: return "[Type-R: SRA]";
        case 0b0000000010: return "[Type-R: SLT]";
        case 0b0000000011: return "[Type-R: SLTU]";
        default: return "[Type-R UNKNOWN]";
    }
}

static const char *LogArithmeticI(
    uint32_t funct3,
    uint32_t funct7
)
{
    switch (funct3)
    {
        case 0b000: return "[Type-I: ADDI]";
        case 0b111: return "[Type-I: ANDI]";
        case 0b110: return "[Type-I: ORI]";
        case 0b100: return "[Type-I: XORI]";
        case 0b010: return "[Type-I: SLTI]";
        case 0b011: return "[Type-I: SLTIU]";
        case 0b001: return "[Type-I: SLLI]";
        case 0b101: return funct7 == 0 ? "[Type-I: SRLI]" : "[Type-I: SRAI]";
        default: return "[Type-I UNKNOWN]";
    }
}

static const char *LogLoadI(
    uint32_t funct3
)
{
    switch (funct3)
    {
        case 0b000: return "[Type-I: LB)]";
        case 0b100: return "[Type-I: LBU)]";
        case 0b001: return "[Type-I: LH]";
        case 0b101: return "[Type-I: LHU]";
        case 0b010: return "[Type-I: LW]";
        default: return "[Type-I UNKNOWN]";
    }
}

static const char *LogStoreI(
    uint32_t funct3
)
{
    switch (funct3)
    {
        case 0b000: return "[Type-S: SB]";
        case 0b001: return "[Type-S: SH]";
        case 0b010: return "[Type-S: SW]";
        default: return "[Type-S UNKNOWN]";
    }
}

static const char *LogBranchB(
    uint32_t funct3
)
{
    switch (funct3)
    {
        case 0b000: return "[Type-B: BEQ]";
        case 0b001: return "[Type-B: BNE]";
        case 0b100: return "[Type-B: BLT]";
        case 0b110: return "[Type-B: BLTU]";
        case 0b101: return "[Type-B: BGE]";
        case 0b111: return "[Type-B: BGEU]";
        default: return "[Type-B UNKNOWN]";
    }
}

static const char *LogBranchU(

)
{
    // TODO
    return "";
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


void LoggerInstruction(
    LOGGING_LEVEL loggingLevel,
    uint32_t opcode,
    uint32_t funct3,
    uint32_t funct7,
    uint32_t rd,
    uint32_t rs1,
    uint32_t rs2,
    uint32_t imm
)
{
    switch (opcode)
    {
    case 0b0110011:
        LoggerMessage(
            loggingLevel, 
            "%s rd=%d rs1=%d rs2=%d", 
            LogArithmeticR(funct3, funct7),
            rd, rs1, rs2
        );
        break;
    case 0b0010011:
        LoggerMessage(
            loggingLevel, 
            "%s rd=%d rs1=%d rs2=%d imm=%d", 
            LogArithmeticI(funct3, funct7),
            rd, rs1, rs2, imm
        );
        break;
    case 0b0000011:
        LoggerMessage(
            loggingLevel, 
            "%s rd=%d rs1=%d imm=%d", 
            LogLoadI(funct3),
            rd, rs1, imm
        );
        break;
    case 0b0100011:
        LoggerMessage(
            loggingLevel, 
            "%s rd=%d rs1=%d imm=%d", 
            LogStoreI(funct3),
            rd, rs1, imm
        );
        break;
    default:
        break;
    }
}