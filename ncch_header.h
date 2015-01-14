#pragma once

#include "common_types.h"

namespace NCCH {

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
    OFFSET_EXHEADER          = 0x200,
};

enum : size_t {
    FLAG_DATA = 0x1,
    FLAG_EXEC = 0x2,

    FLAG_NOROMFS  = 0x2,
    FLAG_NOCRYPTO = 0x4,
};

size_t GetExheaderSize(void* ncch_data) { return *(u32*)(((u8*)ncch_data) + OFFSET_EXHEADER_SIZE); }

size_t GetExefsOffset(void* ncch_data) { return *(u32*)(((u8*)ncch_data) + OFFSET_EXEFS_OFFSET) * 0x200; }
size_t GetExefsSize(void* ncch_data)   { return *(u32*)(((u8*)ncch_data) + OFFSET_EXEFS_SIZE)   * 0x200; }

size_t GetRomfsOffset(void* ncch_data) { return *(u32*)(((u8*)ncch_data) + OFFSET_ROMFS_OFFSET) * 0x200; }
size_t GetRomfsSize(void* ncch_data)   { return *(u32*)(((u8*)ncch_data) + OFFSET_ROMFS_SIZE)   * 0x200; }

size_t GetExefsHashSize(void* ncch_data) { return *(u32*)(((u8*)ncch_data) + OFFSET_EXEFS_HASH_SIZE) * 0x200; }
size_t GetRomfsHashSize(void* ncch_data) { return *(u32*)(((u8*)ncch_data) + OFFSET_ROMFS_HASH_SIZE) * 0x200; }


}

