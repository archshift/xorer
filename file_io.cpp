#include "file_io.h"

std::vector<u8> ReadBinaryFile(const std::string& filename)
{
    FILE* bin_file = fopen(filename.c_str(), "rb");

    if (bin_file == nullptr) return {};

    fseek(bin_file, 0, SEEK_END);
    size_t out_size = ftell(bin_file);
    fseek(bin_file, 0, SEEK_SET);

    std::vector<u8> file_buf(out_size);
    fread(&file_buf[0], 1, file_buf.size(), bin_file);
    fclose(bin_file);
    return file_buf;
};