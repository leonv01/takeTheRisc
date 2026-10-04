#include "risc_memory.h"

#include <string.h>
#include <stdlib.h>

static uint32_t RiscMemoryReadByte(const risc_memory_t *memory, uint32_t address);
static uint32_t RiscMemoryReadWord(const risc_memory_t *memory, uint32_t address);
static uint32_t RiscMemoryReadDWord(const risc_memory_t *memory, uint32_t address);

static void RiscMemoryWriteByte(risc_memory_t *memory, uint32_t address, uint32_t data);
static void RiscMemoryWriteWord(risc_memory_t *memory, uint32_t address, uint32_t data);
static void RiscMemoryWriteDWord(risc_memory_t *memory, uint32_t address, uint32_t data);

int RiscMemoryInitialize(risc_memory_t *memory, uint32_t base, size_t size)
{
    memory->data = (uint8_t *)(malloc(sizeof(uint8_t) * size));

    memset(memory->data, 0, size * sizeof(uint8_t));

    memory->size = size;
    memory->base = base;

    return 0;
}

int RiscMemoryDestroy(risc_memory_t *memory)
{
    free(memory->data);
    memory->data = NULL;
    memory->size = 0;

    return 0;
}

int RiscMemoryRead(const risc_memory_t *memory, uint32_t address, DATA_SIZE dataSize, uint32_t *data)
{
    switch (dataSize)
    {
        case BYTE: *data = RiscMemoryReadByte(memory, address); break;
        case WORD: *data = RiscMemoryReadWord(memory, address); break;
        case DWORD: *data = RiscMemoryReadDWord(memory, address); break;
        default:
            break;
    }
    return 0;
}

int RiscMemoryWrite(risc_memory_t *memory, uint32_t address, DATA_SIZE dataSize, uint32_t data)
{
    switch (dataSize) 
    {
        case BYTE: RiscMemoryWriteByte(memory, address, data); break;
        case WORD: RiscMemoryWriteWord(memory, address, data); break;
        case DWORD: RiscMemoryWriteDWord(memory, address, data); break;
        default:
            break;
    }
    return 0;
}

uint32_t RiscMemoryReadByte(const risc_memory_t *memory, uint32_t address)
{    
    uint8_t *ptr = &memory->data[address];

    return  (uint32_t) (
            (uint32_t) *(ptr) << 0
    );
}

uint32_t RiscMemoryReadWord(const risc_memory_t *memory, uint32_t address)
{    
    uint8_t *ptr = &memory->data[address];

    return  (uint32_t) (
            (uint32_t) *(ptr) << 0
        |   (uint32_t) (*(ptr + 1)) << 8
    );
}

uint32_t RiscMemoryReadDWord(const risc_memory_t *memory, uint32_t address)
{
    uint8_t *ptr = &memory->data[address];

    return  (uint32_t) (
            (uint32_t) (*(ptr) << 0)
        |   (uint32_t) (*(ptr + 1)) << 8
        |   (uint32_t) (*(ptr + 2)) << 16 
        |   (uint32_t) (*(ptr + 3)) << 24
    );
}

void RiscMemoryWriteByte(risc_memory_t *memory, uint32_t address, uint32_t data)
{
    uint8_t *ptr = &memory->data[address];

    *ptr = (uint8_t) (data & 0xFF);
}

void RiscMemoryWriteWord(risc_memory_t *memory, uint32_t address, uint32_t data)
{
    uint8_t *ptr = &memory->data[address];

    *ptr = (uint8_t) (data & 0xFF);
    *(ptr + 1) = (uint8_t) ((data >> 8) & 0xFF);
}

void RiscMemoryWriteDWord(risc_memory_t *memory, uint32_t address, uint32_t data)
{
    uint8_t *ptr = &memory->data[address];

    *ptr = (uint8_t) (data & 0xFF);
    *(ptr + 1) = (uint8_t) ((data >> 8) & 0xFF);
    *(ptr + 2) = (uint8_t) ((data >> 16) & 0xFF);
    *(ptr + 3) = (uint8_t) ((data >> 24) & 0xFF);
}
