#include "ncch.h"

#include "common_funcs.h"

bool NCCH::DecryptExheader(const std::vector<u8> &xorpad)
{
    if (xorpad.empty()) {
        printf("ERROR: Input exheader XORPad does not exist!\n");
        return false;
    }

    u8* exheader = &file_buf[NCCH::OFFSET_EXHEADER];
    if (xorpad.size() < 0x800) {
        printf("ERROR: Exheader XORPad invalid!\n");
        return false;
    }

    XOR(exheader, &xorpad[0], 0x800);
    if (!CompareHash(exheader, 0x400, &file_buf[NCCH_Header::OFFSET_EXHEADER_HASH])) {
        printf("ERROR: Exheader XORPad invalid!\n");
        return false;
    }
    return true;
}

bool NCCH::DecryptEXEFS(const std::vector<u8> &xorpad)
{
    if (xorpad.empty()) {
        printf("ERROR: Input EXEFS XORPad does not exist!\n");
        return false;
    }

    u8* exefs = &file_buf[header.GetExefsOffset()];
    if (xorpad.size() < header.GetExefsSize()) {
        printf("ERROR: EXEFS XORPad invalid!\n");
        return false;
    }

    XOR(exefs, &xorpad[0], header.GetExefsSize());

    if (!CompareHash(exefs, header.GetExefsHashSize(), &file_buf[NCCH_Header::OFFSET_EXEFS_HASH])) {
        printf("ERROR: EXEFS XORPad invalid!\n");
        return false;
    }
    return true;
}

bool NCCH::DecryptROMFS(const std::vector<u8> &xorpad)
{
    if (xorpad.empty()) {
        printf("ERROR: Input ROMFS XORPad does not exist!\n");
        return false;
    }

    u8* romfs = &file_buf[header.GetRomfsOffset()];
    if (xorpad.size() < header.GetRomfsSize()) {
        printf("ERROR: ROMFS XORPad invalid!\n");
        return false;
    }

    XOR(romfs, &xorpad[0], header.GetRomfsSize());

    if (!CompareHash(romfs, header.GetRomfsHashSize(), &file_buf[NCCH_Header::OFFSET_ROMFS_HASH])) {
        printf("ERROR: ROMFS XORPad invalid!\n");
        return false;
    }
    return true;
}