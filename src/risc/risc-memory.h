#ifndef RISC_MEMORY_H__
#define RISC_MEMORY_H__

#include <stdint.h>
#include <stddef.h>

#define RISC_MEMORY_DEFAULT_SIZE (1u << 20) // 1 MiB

typedef struct risc_memory_t
{
    uint8_t *data;
    size_t size;
    uint32_t base;
} risc_memory_t;

int RiscMemoryInitialize(risc_memory_t *memory, uint32_t base, size_t size);
int RiscMemoryDestroy(risc_memory_t *memory);

int RiscMemoryRead(const risc_memory_t *memory, uint32_t address, size_t width, uint32_t *data);
int RiscMemoryWrite(risc_memory_t *memory, uint32_t address, size_t width, uint32_t data);

#endif // RISC_MEMORY_H__