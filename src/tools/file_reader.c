#include "tools/file_reader.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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