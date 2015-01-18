#pragma once

#include <vector>
#include <string>

#include "common_types.h"

size_t WriteBinaryFile(const std::string& filename, const void* buffer, const size_t buf_size);
std::vector<u8> ReadBinaryFile(const std::string& filename);

std::string ReplaceExtension(const std::string& filename, const std::string& ext);
