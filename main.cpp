#include <string>
#include <cstdlib>
#include <vector>

#include "file_io.h"
#include "common_types.h"
#include "ncch.h"

int main(int argc, char** argv)
{
    if (argc < 4) {
        printf("Usage: xorer file{.app|.cxi} exheader.xorpad exefs.xorpad [romfs.xorpad]\n");
        return -1;
    }

    std::vector<u8> app_file = ReadBinaryFile(argv[1]);
    if (app_file.empty()) {
        printf("ERROR: Input file does not exist!\n");
        return -1;
    }

    if (memcmp(&app_file[NCCH_Header::OFFSET_MAGIC], "NCCH", 4) != 0) {
        printf("Non-NCCH files not yet supported!");
        return -1;
    }

    NCCH ncch(app_file);

    u8 data_format_flag = app_file[NCCH_Header::OFFSET_FLAG_DATA_FORMAT];
    if ((data_format_flag & NCCH_Header::FLAG_DATA) && !(data_format_flag & NCCH_Header::FLAG_EXEC)) {
        // File is a CFA
        printf("Filetype: CFA\n");
        printf("CFA not yet supported!\n");
        return -1;
    } else {
        // File is a CXI
        printf("Filetype: CXI\n");

        if (!ncch.DecryptExheader(ReadBinaryFile(argv[2]))) return -1;
        if (!ncch.DecryptEXEFS(ReadBinaryFile(argv[3]))) return -1;

        if (data_format_flag & NCCH_Header::FLAG_NOROMFS) {
            if (argc != 4) {
                printf("Usage for this filetype: xorer file{.app|.cxi} exheader.xorpad exefs.xorpad\n");
                return -1;
            }
        } else {
            if (argc != 5) {
                printf("Usage for this filetype: xorer file{.app|.cxi} exheader.xorpad exefs.xorpad romfs.xorpad\n");
                return -1;
            }

            if (!ncch.DecryptROMFS(ReadBinaryFile(argv[4]))) return -1;
        }

        ncch.SetDecrypted();
        WriteBinaryFile(ReplaceExtension(argv[1], "cxi"), app_file);
    }

    return 0;
}
