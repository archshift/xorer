#include <string>
#include <cinttypes>
#include <cstdlib>
#include <vector>

#include "polarssl/sha256.h"

#include "ncch_header.h"
#include "file_io.h"
#include "common_types.h"

void XOR(u8* target_buf, const u8* xorpad, const size_t size)
{
    for (size_t byte = 0; byte < size; ++byte) {
        target_buf[byte] ^= xorpad[byte];
    }
}

bool CompareHash(const u8* data_buf, const size_t data_size, const u8* hash_buf)
{
    std::unique_ptr<u8> generated_hash(new u8[32]);
    sha256(data_buf, data_size, generated_hash.get(), 0);
    return memcmp(generated_hash.get(), hash_buf, 32) == 0;
}

bool CXIDecryptExheader(u8* file_buf, const std::vector<u8>& xorpad)
{
    if (xorpad.empty()) {
        printf("ERROR: Input exheader XORPad does not exist!\n");
        return false;
    }

    u8* exheader = file_buf + NCCH::OFFSET_EXHEADER;
    XOR(exheader, &xorpad[0], 0x800);
    if (!CompareHash(exheader, 0x400, file_buf + NCCH::OFFSET_EXHEADER_HASH)) {
        printf("ERROR: Exheader XORPad invalid!");
        return false;
    }
    return true;
}

bool CXIDecryptEXEFS(u8* file_buf, const std::vector<u8>& xorpad)
{
    if (xorpad.empty()) {
        printf("ERROR: Input EXEFS XORPad does not exist!\n");
        return false;
    }

    u8* exefs = file_buf + NCCH::GetExefsOffset(file_buf);
    XOR(exefs, &xorpad[0], NCCH::GetExefsSize(file_buf));

    if (!CompareHash(exefs, NCCH::GetExefsHashSize(file_buf), file_buf + NCCH::OFFSET_EXEFS_HASH)) {
        printf("ERROR: EXEFS XORPad invalid!");
        return false;
    }
    return true;
}

bool CXIDecryptROMFS(u8* file_buf, const std::vector<u8>& xorpad)
{
    if (xorpad.empty()) {
        printf("ERROR: Input ROMFS XORPad does not exist!\n");
        return false;
    }

    u8* romfs = file_buf + NCCH::GetRomfsOffset(file_buf);
    XOR(romfs, &xorpad[0], NCCH::GetRomfsSize(file_buf));

    if (!CompareHash(romfs, NCCH::GetRomfsHashSize(file_buf), file_buf + NCCH::OFFSET_ROMFS_HASH)) {
        printf("ERROR: ROMFS XORPad invalid!");
        return false;
    }
    return true;
}

int main(int argc, char** argv)
{
    if (argc < 4) {
        printf("Usage: xorer file{.app|.cxi} exheader.xorpad exefs.xorpad [romfs.xorpad]\n");
        return -1;
    }

    std::vector<u8> app_file = ReadBinaryFile(argv[1]);
    if (app_file.empty()) {
        printf("ERROR: Input NCCH does not exist!\n");
        return -1;
    }

    bool confirm_magic = memcmp(&app_file[NCCH::OFFSET_MAGIC], "NCCH", 4) == 0;
    printf("Magic confirmed: %s\n", confirm_magic ? "true" : "false");

    u8 data_format_flag = app_file[NCCH::OFFSET_FLAG_DATA_FORMAT];
    if ((data_format_flag & NCCH::FLAG_DATA) && !(data_format_flag & NCCH::FLAG_EXEC)) {
        // File is a CFA
        printf("Filetype: CFA\n");
        printf("CFA not yet supported!\n");
        return -1;
    } else {
        // File is a CXI
        printf("Filetype: CXI\n");

        if (!CXIDecryptExheader(&app_file[0], ReadBinaryFile(argv[2]))) return -1;
        if (!CXIDecryptEXEFS(&app_file[0], ReadBinaryFile(argv[3]))) return -1;

        if (data_format_flag & NCCH::FLAG_NOROMFS) {
            if (argc != 4) {
                printf("Usage for this filetype: xorer file{.app|.cxi} exheader.xorpad exefs.xorpad\n");
                return -1;
            }
        } else {
            if (argc != 5) {
                printf("Usage for this filetype: xorer file{.app|.cxi} exheader.xorpad exefs.xorpad romfs.xorpad\n");
                return -1;
            }

            if (!CXIDecryptROMFS(&app_file[0], ReadBinaryFile(argv[4]))) return -1;
        }

        app_file[NCCH::OFFSET_FLAG_CRYPTO] = 0;
        app_file[NCCH::OFFSET_FLAG_DATA_FORMAT] |= NCCH::FLAG_NOCRYPTO;
        WriteBinaryFile(std::string(argv[1]) + ".cxi", app_file);
    }

    return 0;
}
