#include "tools/file_reader.h"
#include "risc/risc_cpu.h"
#include "tools/logger.h"

#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <elf.h>

void FileReaderRead(const char *path, uint8_t *buffer, size_t size)
{
    FILE *filePtr;

    filePtr = fopen(path, "r");

    if (filePtr == NULL)
    {
        printf("Error opening file\n");
    }
    else
    {
        memset(buffer, 0, size);

        fread(buffer, sizeof(uint8_t), size, filePtr);
        fclose(filePtr);
    }
}

size_t FileReaderLoadMemory(const char *path, risc_memory_t *memory, uint32_t address)
{
    FILE *filePtr;

    filePtr = fopen(path, "rb");

    if (filePtr == NULL)
    {
        printf("Error opening file\n");
    }
    else
    {
        fseek(filePtr, 0, SEEK_END);
        size_t fileLength = ftell(filePtr);
        fseek(filePtr, 0, SEEK_SET);

        if (fileLength <= 0)
        {
            fclose(filePtr);
            return 0;
        }
        
        uint32_t offset = address;

        if (address >= memory->base)
        {
            offset = address - memory->base;
        }

        if (offset + (size_t)fileLength > memory->size)
        {
            fclose(filePtr);
            return 0;
        }

        size_t byteRead = fread(&memory->data[offset], sizeof(uint8_t), fileLength, filePtr);
        fclose(filePtr);
        
        return byteRead;
    }
}

void FileReaderLoadMemoryElf(const char *path, risc_cpu_t *cpu)
{
    Elf32_Ehdr header;
    
    LoggerMessage(INFO, "[%s] Opening file: %s", __FUNCTION__, path);

    FILE *file = fopen(path, "rb");

    if (file)
    {
        fread(&header, sizeof(header), 1, file);

        if (memcmp(header.e_ident, ELFMAG, SELFMAG) == 0)
        {
            LoggerMessage(SUCCESS, "[%s] ELF file contains MAGIC (0x%.2X)", __FUNCTION__, ELFMAG0);

            LoggerMessage(INFO, "[%s] Setting program counter to e_entry (0x%.8X)", __FUNCTION__, header.e_entry);
            cpu->pc = header.e_entry;

            LoggerMessage(DEBUG, "[%s] Iterating through program header", __FUNCTION__);
            for (size_t i = 0; i < header.e_phnum; i++) 
            {
                LoggerMessage(DEBUG, "[%s] Program header entry: %ld", __FUNCTION__, i);
                fseek(file, header.e_phoff + i * sizeof(Elf32_Phdr), SEEK_SET);
                
                Elf32_Phdr programHeader;
                fread(&programHeader, sizeof(Elf32_Phdr), 1, file);
                
                LoggerMessage(DEBUG, "[%s] Program header p_type 0x%.2x", __FUNCTION__, programHeader.p_type);

                // Find loadable segments in the program header
                if (programHeader.p_type == PT_LOAD)
                {
                    fseek(file, programHeader.p_offset, SEEK_SET);
                    uint32_t memoryOffset = programHeader.p_paddr - cpu->memory->base;
                    fread(&cpu->memory->data[memoryOffset], 1, programHeader.p_filesz, file);
                    LoggerMessage(DEBUG, "[%s] Found loadable segment in program header. Loading into memory (0x%.8x)", __FUNCTION__, memoryOffset);
                }
            }
        }
        else
        {
            LoggerMessage(ERROR, "[%s] ELF file does not contain MAGIC (0x%.2X)", __FUNCTION__, ELFMAG0);
        }

        fclose(file);
    }
}