#include <string>
#include <cinttypes>
#include <cstdlib>
#include <vector>

#include "ncch_header.h"
#include "file_io.h"
#include "common_types.h"

void XOR(u8* target_buf, const u8* xorpad, const size_t size)
{
    for (size_t byte = 0; byte < size; ++byte) {
        target_buf[byte] ^= xorpad[byte];
    }
}

void DecryptCXI(u8* app_file_buf, const u8* exhead_xorpad, const u8* exefs_xorpad)
{
    size_t exhead_size  = *reinterpret_cast<u32*>(app_file_buf + NCCH_OFFSET_EXHEADER_SIZE);
    XOR(app_file_buf + NCCH_OFFSET_EXHEADER, exhead_xorpad, exhead_size);

    size_t exefs_offset = *reinterpret_cast<u32*>(app_file_buf + NCCH_OFFSET_EXEFS_OFFSET) * 0x200;
    size_t exefs_size   = *reinterpret_cast<u32*>(app_file_buf + NCCH_OFFSET_EXEFS_SIZE)   * 0x200;
    XOR(app_file_buf + exefs_offset, exefs_xorpad, exefs_size);

    app_file_buf[NCCH_OFFSET_FLAG_CRYPTO] = 0;
    app_file_buf[NCCH_OFFSET_FLAG_DATA_FORMAT] |= FLAG_NOCRYPTO;
}

void DecryptCXI(u8* app_file_buf, const u8* exhead_xorpad, const u8* exefs_xorpad, const u8* romfs_xorpad)
{
    size_t exhead_size  = *reinterpret_cast<u32*>(app_file_buf + NCCH_OFFSET_EXHEADER_SIZE);
    XOR(app_file_buf + NCCH_OFFSET_EXHEADER, exhead_xorpad, exhead_size);

    size_t exefs_offset = *reinterpret_cast<u32*>(app_file_buf + NCCH_OFFSET_EXEFS_OFFSET) * 0x200;
    size_t exefs_size   = *reinterpret_cast<u32*>(app_file_buf + NCCH_OFFSET_EXEFS_SIZE)   * 0x200;
    XOR(app_file_buf + exefs_offset, exefs_xorpad, exefs_size);

    size_t romfs_offset = *reinterpret_cast<u32*>(app_file_buf + NCCH_OFFSET_ROMFS_OFFSET) * 0x200;
    size_t romfs_size   = *reinterpret_cast<u32*>(app_file_buf + NCCH_OFFSET_ROMFS_SIZE)   * 0x200;
    XOR(app_file_buf + romfs_offset, romfs_xorpad, romfs_size);

    app_file_buf[NCCH_OFFSET_FLAG_CRYPTO] = 0;
    app_file_buf[NCCH_OFFSET_FLAG_DATA_FORMAT] |= FLAG_NOCRYPTO;
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

    bool confirm_magic = memcmp(&app_file[NCCH_OFFSET_MAGIC], "NCCH", 4) == 0;
    printf("Magic confirmed: %s\n", confirm_magic ? "true" : "false");

    u8 data_format_flag = app_file[NCCH_OFFSET_FLAG_DATA_FORMAT];
    if ((data_format_flag & FLAG_DATA) && !(data_format_flag & FLAG_EXEC)) {
        // File is a CFA
        printf("Filetype: CFA\n");
        printf("CFA not yet supported!\n");
        return -1;
    } else {
        // File is a CXI
        printf("Filetype: CXI\n");

        std::vector<u8> exhead_xorpad = ReadBinaryFile(argv[2]);
        if (exhead_xorpad.empty()) {
            printf("ERROR: Input exheader XORPad does not exist!\n");
            return -1;
        }
        std::vector<u8> exefs_xorpad  = ReadBinaryFile(argv[3]);
        if (exefs_xorpad.empty()) {
            printf("ERROR: Input EXEFS XORPad does not exist!\n");
            return -1;
        }

        if (data_format_flag & FLAG_NOROMFS) {
            if (argc != 4) {
                printf("Usage for this filetype: xorer file{.app|.cxi} exheader.xorpad exefs.xorpad\n");
                return -1;
            }
            DecryptCXI(&app_file[0], &exhead_xorpad[0], &exefs_xorpad[0]);
        } else {
            if (argc != 5) {
                printf("Usage for this filetype: xorer file{.app|.cxi} exheader.xorpad exefs.xorpad romfs.xorpad\n");
                return -1;
            }

            std::vector<u8> romfs_xorpad = ReadBinaryFile(argv[4]);
            if (romfs_xorpad.empty()) {
                printf("ERROR: Input ROMFS XORPad does not exist!\n");
                return -1;
            }

            DecryptCXI(&app_file[0], &exhead_xorpad[0], &exefs_xorpad[0], &romfs_xorpad[0]);
        }
        WriteBinaryFile(std::string(argv[1]) + ".cxi", app_file);
    }

    return 0;
}
