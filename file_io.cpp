#include "file_io.h"
#include <cstdio>

size_t WriteBinaryFile(const std::string& filename, const void* buffer, const size_t buf_size)
{
    FILE* file = fopen(filename.c_str(), "wb");
    size_t size_written = fwrite(buffer, 1, buf_size, file);
    fclose(file);
    return size_written;
}

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

std::string ReplaceExtension(const std::string& filename, const std::string& ext) {
    std::string new_filename;
    size_t dot_pos = filename.find_last_of('.');
    if (dot_pos != std::string::npos)
    new_filename = filename.substr(0, dot_pos);
    new_filename += '.' + ext;
    return new_filename;
}
