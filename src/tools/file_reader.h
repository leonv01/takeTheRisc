#ifndef FILE_READER_H__
#define FILE_READER_H__

#include <stdint.h>
#include <stdlib.h>

void FileReaderRead(const char *path, uint8_t *buffer, size_t size);

#endif // FILE_READER_H__