#ifndef FILE_READER_H__
#define FILE_READER_H__

#include <stdint.h>
#include <stdlib.h>

#include "risc/risc_cpu.h"

void FileReaderRead(const char *path, uint8_t *buffer, size_t size);
size_t FileReaderLoadMemory(const char *path, risc_memory_t *memory, uint32_t address);

void FileReaderLoadMemoryElf(const char *path, risc_cpu_t *cpu);

#endif // FILE_READER_H__