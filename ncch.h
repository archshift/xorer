#pragma once

#include <vector>

#include "common_types.h"

class NCCH_Header {
    const std::vector<u8> header;
public:
    enum : size_t {
        OFFSET_MAGIC             = 0x100,
        OFFSET_EXHEADER_HASH     = 0x160,
        OFFSET_EXHEADER_SIZE     = 0x180,
        OFFSET_FLAGS             = 0x188,
        OFFSET_FLAG_CRYPTO       = OFFSET_FLAGS + 3,
        OFFSET_FLAG_CONTENT_TYPE = OFFSET_FLAGS + 5,
        OFFSET_FLAG_DATA_FORMAT  = OFFSET_FLAGS + 7,
        OFFSET_EXEFS_OFFSET      = 0x1A0,
        OFFSET_EXEFS_SIZE        = 0x1A4,
        OFFSET_EXEFS_HASH_SIZE   = 0x1A8,
        OFFSET_ROMFS_OFFSET      = 0x1B0,
        OFFSET_ROMFS_SIZE        = 0x1B4,
        OFFSET_ROMFS_HASH_SIZE   = 0x1B8,
        OFFSET_EXEFS_HASH        = 0x1C0,
        OFFSET_ROMFS_HASH        = 0x1E0,

        FLAG_DATA = 0x1,
        FLAG_EXEC = 0x2,
        FLAG_NOROMFS  = 0x2,
        FLAG_NOCRYPTO = 0x4,
    };

    NCCH_Header(const std::vector<u8>& ncch_data):
        header(ncch_data.begin(), ncch_data.begin() + 0x200) { }

    size_t GetExheaderSize()  { return *(u32*)(&header[OFFSET_EXHEADER_SIZE]); }

    size_t GetExefsOffset()   { return *(u32*)(&header[OFFSET_EXEFS_OFFSET]) * 0x200; }
    size_t GetExefsSize()     { return *(u32*)(&header[OFFSET_EXEFS_SIZE])   * 0x200; }

    size_t GetRomfsOffset()   { return *(u32*)(&header[OFFSET_ROMFS_OFFSET]) * 0x200; }
    size_t GetRomfsSize()     { return *(u32*)(&header[OFFSET_ROMFS_SIZE])   * 0x200; }

    size_t GetExefsHashSize() { return *(u32*)(&header[OFFSET_EXEFS_HASH_SIZE]) * 0x200; }
    size_t GetRomfsHashSize() { return *(u32*)(&header[OFFSET_ROMFS_HASH_SIZE]) * 0x200; }
};


class NCCH {
    std::vector<u8> file_buf;
    NCCH_Header header;
public:
    enum : size_t {
        OFFSET_HEADER   = 0x0,
        OFFSET_EXHEADER = 0x200,
        OFFSET_DATA     = 0xA00,
    };

    NCCH(std::vector<u8> file): file_buf(file), header(file_buf) {}

    bool DecryptExheader(const std::vector<u8>& xorpad);
    bool DecryptEXEFS(const std::vector<u8>& xorpad);
    bool DecryptROMFS(const std::vector<u8>& xorpad);

    const std::vector<u8>& GetBuffer() { return file_buf; }

    void SetDecrypted() {
        file_buf[NCCH_Header::OFFSET_FLAG_CRYPTO] = 0;
        file_buf[NCCH_Header::OFFSET_FLAG_DATA_FORMAT] |= NCCH_Header::FLAG_NOCRYPTO;
    }
};