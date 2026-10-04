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