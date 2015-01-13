#pragma once

#include <vector>
#include <string>

#include "common_types.h"

template <typename T>
size_t WriteBinaryFile(const std::string& filename, const std::vector<T>& buffer)
{
    FILE* file = fopen(filename.c_str(), "wb");
    size_t size_written = fwrite(&buffer[0], 1, buffer.size(), file);
    fclose(file);
    return size_written;
}

std::vector<u8> ReadBinaryFile(const std::string& filename);