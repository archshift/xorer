#pragma once

#include <vector>
#include <array>
#include <cstring>

#include "common_types.h"
#include "common_funcs.h"

class NCCH_Header {
    const u8* header;
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

    NCCH_Header(u8* ncch_data):
        header(ncch_data) { }

    size_t GetExheaderSize()  { return *(u32*)(&header[OFFSET_EXHEADER_SIZE]); }

    size_t GetExefsOffset()   { return *(u32*)(&header[OFFSET_EXEFS_OFFSET]) * 0x200; }
    size_t GetExefsSize()     { return *(u32*)(&header[OFFSET_EXEFS_SIZE])   * 0x200; }

    size_t GetRomfsOffset()   { return *(u32*)(&header[OFFSET_ROMFS_OFFSET]) * 0x200; }
    size_t GetRomfsSize()     { return *(u32*)(&header[OFFSET_ROMFS_SIZE])   * 0x200; }

    size_t GetExefsHashSize() { return *(u32*)(&header[OFFSET_EXEFS_HASH_SIZE]) * 0x200; }
    size_t GetRomfsHashSize() { return *(u32*)(&header[OFFSET_ROMFS_HASH_SIZE]) * 0x200; }

    u8 GetDataFormatBitmask() { return header[OFFSET_FLAG_DATA_FORMAT]; }
};

class EXEFS {
    u8* buffer;

    typedef u8 FileHash[32];
public:
#pragma pack(1)
    struct FileHeader {
        char filename[8];
        u32 file_offset;
        u32 file_size;
    };

    struct EXEFS_Header {
        FileHeader headers[10];
        INSERT_PADDING(0x20);
        FileHash hashes[10];
    } header;
#pragma pack()

    EXEFS(u8* buffer): buffer(buffer)
    {
        memcpy(&header, buffer, 0x200);
    }

    FileHeader GetCodeHeader()
    {
        for (int i = 0; i < 10; ++i) {
            const char* header_filename = header.headers[i].filename;
            if (memcmp(header_filename, ".code", 5) == 0)
                return header.headers[i];
        }
        return {};
    }

    bool VerifyHashes()
    {
        for (int i = 0; i < 10; ++i) {
            static const u8 zero_header[16] = {0};
            static const u8 zero_hash[32] = {0};
            // We if the header and the hash are both zero-initialized, there's no code to verify
            if (memcmp(&header.headers[i], zero_header, 16) == 0 && memcmp(header.hashes[i], zero_hash, 32) == 0)
                return true;

            // For some reason, the hashes are in reverse order...
            if (!CompareHash(buffer + 0x200 + header.headers[i].file_offset,
                             header.headers[i].file_size, header.hashes[9-i]))
                return false;
        }
        return true;
    }
};


class NCCH {
    u8* buffer;
    size_t size;
    NCCH_Header header;
public:
    enum ContainerType {
        TYPE_CXI,
        TYPE_CFA
    };

    enum : size_t {
        OFFSET_HEADER   = 0x0,
        OFFSET_EXHEADER = 0x200,
        OFFSET_DATA     = 0xA00,
    };

    NCCH(u8* buf, size_t size): buffer(buf), size(size), header(buffer) {}

    bool DecryptExheader(const std::vector<u8>& xorpad);
    bool DecryptEXEFS(const std::vector<u8>& xorpad);
    bool DecryptEXEFS(const std::vector<u8>& xorpad_normal, const std::vector<u8>& xorpad_7x);
    bool DecryptROMFS(const std::vector<u8>& xorpad);

    const u8* GetBuffer() { return buffer; }
    const size_t GetSize() { return size; }

    const ContainerType GetType();

    const bool HasRomFS() { return (header.GetDataFormatBitmask() & NCCH_Header::FLAG_NOROMFS) == 0; }

    void SetDecrypted() {
        buffer[NCCH_Header::OFFSET_FLAG_CRYPTO] = 0;
        buffer[NCCH_Header::OFFSET_FLAG_DATA_FORMAT] |= NCCH_Header::FLAG_NOCRYPTO;
    }
};
