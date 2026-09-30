#include "risc-memory.h"

#include <string.h>

int RiscMemoryInitialize(risc_memory_t *memory, uint32_t base, size_t size)
{
    memory->data = (uint8_t *)(malloc(sizeof(uint8_t) * size));

    memset(memory->data, size * sizeof(uint8_t), 0);

    memory->size = size;
    memory->base = base;
}

int RiscMemoryDestroy(risc_memory_t *memory)
{
    free(memory->data);
    memory->data = NULL;
    memory->size = 0;
}

int RiscMemoryRead(const risc_memory_t *memory, uint32_t address, size_t width, uint32_t *data)
{
}

int RiscMemoryWrite(risc_memory_t *memory, uint32_t address, size_t width, uint32_t data)
{

}
