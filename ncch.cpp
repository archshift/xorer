#include "ncch.h"

#include "common_funcs.h"

bool NCCH::DecryptExheader(const std::vector<u8>& xorpad)
{
    if (xorpad.empty()) {
        printf("ERROR: Input exheader XORPad does not exist!\n");
        return false;
    }

    u8* exheader = &buffer[NCCH::OFFSET_EXHEADER];
    if (xorpad.size() < 0x800) {
        printf("ERROR: Exheader XORPad invalid!\n");
        return false;
    }

    XOR(exheader, xorpad.data(), 0x800);
    if (!CompareHash(exheader, 0x400, &buffer[NCCH_Header::OFFSET_EXHEADER_HASH])) {
        printf("ERROR: Exheader XORPad invalid!\n");
        return false;
    }
    return true;
}

bool NCCH::DecryptEXEFS(const std::vector<u8>& xorpad)
{
    if (xorpad.empty()) {
        printf("ERROR: Input EXEFS XORPad does not exist!\n");
        return false;
    }

    u8* exefs = &buffer[header.GetExefsOffset()];
    if (xorpad.size() < header.GetExefsSize()) {
        printf("ERROR: EXEFS XORPad invalid!\n");
        return false;
    }

    XOR(exefs, &xorpad[0], header.GetExefsSize());

    if (!CompareHash(exefs, header.GetExefsHashSize(), &buffer[NCCH_Header::OFFSET_EXEFS_HASH])) {
        printf("ERROR: EXEFS XORPad invalid!\n");
        return false;
    }
    return true;
}

bool NCCH::DecryptEXEFS(const std::vector<u8>& xorpad_normal, const std::vector<u8>& xorpad_7x)
{
    if (xorpad_normal.empty()) {
        printf("ERROR: Input EXEFS normal XORPad does not exist!\n");
        return false;
    }

    if (xorpad_7x.empty()) {
        printf("ERROR: Input EXEFS 7.x XORPad does not exist!\n");
        return false;
    }

    u8* exefs_buf = &buffer[header.GetExefsOffset()];
    if (xorpad_normal.size() + xorpad_7x.size() < header.GetExefsSize()) {
        printf("ERROR: EXEFS XORPads invalid!\n");
        return false;
    }

    // Decrypt the EXEFS header so we can find the .code section
    XOR(exefs_buf, &xorpad_normal[0], 0x200);

    EXEFS exefs(exefs_buf);
    EXEFS::FileHeader code_file = exefs.GetCodeHeader();
    size_t code_begin_offset = 0x200 + code_file.file_offset;
    size_t code_end_offset   = code_begin_offset + code_file.file_size;

    // Everything up to the start of the code section
    // (excluding the header, which is already decrypted)
    XOR(exefs_buf + 0x200, &xorpad_normal[0] + 0x200, code_begin_offset - 0x200);
    // Everything in the code section
    XOR(exefs_buf + code_begin_offset, &xorpad_7x[0] + code_begin_offset, code_file.file_size);
    // Everything after the end of the code section
    XOR(exefs_buf + code_end_offset, &xorpad_normal[0] + code_end_offset, header.GetExefsSize() - code_end_offset);

    if (!exefs.VerifyHashes() ||
        !CompareHash(exefs_buf, header.GetExefsHashSize(), &buffer[NCCH_Header::OFFSET_EXEFS_HASH])) {
        printf("ERROR: EXEFS XORPads invalid!\n");
        return false;
    }
    return true;
}

bool NCCH::DecryptROMFS(const std::vector<u8>& xorpad)
{
    if (xorpad.empty()) {
        printf("ERROR: Input ROMFS XORPad does not exist!\n");
        return false;
    }

    u8* romfs = &buffer[header.GetRomfsOffset()];
    if (xorpad.size() < header.GetRomfsSize()) {
        printf("ERROR: ROMFS XORPad invalid!\n");
        return false;
    }

    XOR(romfs, &xorpad[0], header.GetRomfsSize());

    if (!CompareHash(romfs, header.GetRomfsHashSize(), &buffer[NCCH_Header::OFFSET_ROMFS_HASH])) {
        printf("ERROR: ROMFS XORPad invalid!\n");
        return false;
    }
    return true;
}

const NCCH::ContainerType NCCH::GetType()
{
    u8 content_type_flag = buffer[NCCH_Header::OFFSET_FLAG_CONTENT_TYPE];
    if ((content_type_flag & NCCH_Header::FLAG_DATA) && !(content_type_flag & NCCH_Header::FLAG_EXEC))
        return TYPE_CFA;
    else
        return TYPE_CXI;
}
