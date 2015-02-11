#include <string>
#include <cstdlib>
#include <vector>
#include <map>

#ifdef _MSC_VER
// Required define for MSVC compatibility
#define __STDC__ 1
#endif

#include "getopt/getopt.h"

#include "file_io.h"
#include "common_types.h"
#include "ncch.h"
#include "ncsd.h"

typedef std::map<std::string, const char*> optlist;
typedef std::vector<std::string> flaglist;

bool DecryptCXI(const optlist& args, NCCH* ncch)
{
    if (!Found(args, "exefs") && !Found(args, "exefs7")) {
        printf("ERROR: 7.x EXEFS XORPads must be accompanied by normal EXEFS XORPads!\n");
        return false;
    }

    if (!Found(args, "exheader")) {
        printf("ERROR: The input file type requires an exheader xorpad!\n");
        return false;
    }

    if (!ncch->DecryptExheader(ReadBinaryFile(args.at("exheader"))))
        return false;

    if (!Found(args, "exefs")) {
        printf("ERROR: CXIs without EXEFSs not yet supported!\n");
        return false;
    }

    if (!Found(args, "exefs7")) {
        if (!ncch->DecryptEXEFS(ReadBinaryFile(args.at("exefs"))))
            return false;
    } else if (!ncch->DecryptEXEFS(ReadBinaryFile(args.at("exefs")), ReadBinaryFile(args.at("exefs7")))) {
        return false;
    }

    if (ncch->HasRomFS()) {
        if (!Found(args, "romfs")) {
            printf("ERROR: The input file type requires a ROMFS xorpad!\n");
            return false;
        }
        if (!ncch->DecryptROMFS(ReadBinaryFile(args.at("romfs"))))
            return false;
    }

    ncch->SetDecrypted();
    return true;
}

bool DecryptCFA(const optlist& args, NCCH* ncch)
{
    if (Found(args, "exefs") || Found(args, "exefs7")) {
        printf("ERROR: CFAs with EXEFSs not yet supported!\n");
        return false;
    }

    if (ncch->HasRomFS()) {
        if (!Found(args, "romfs")) {
            printf("ERROR: The input file type requires a ROMFS xorpad!\n");
            return false;
        }
        if (!ncch->DecryptROMFS(ReadBinaryFile(args.at("romfs"))))
            return false;
    }

    ncch->SetDecrypted();
    return true;
}

bool DecryptNCCH(const optlist& args, NCCH* ncch)
{
    switch(ncch->GetType()) {
        case NCCH::TYPE_CXI: return DecryptCXI(args, ncch);
        case NCCH::TYPE_CFA: return DecryptCFA(args, ncch);
    }
    return false;
}

void ShowHelpInfo()
{
    printf("xorer: Apply XORPads to encrypted 3DS files\n");
    printf("Usage: xorer <file> [-p num] [-e xorpad] [-x xorpad] [-r xorpad] [-7 xorpad]\n");
    printf("  -h  --help        Display this help information\n");
    printf("  -e  --exheader    Specify the Exheader XORPad\n");
    printf("  -x  --exefs       Specify the (normal) EXEFS Xorpad\n");
    printf("  -r  --romfs       Specify the ROMFS Xorpad\n");
    printf("  -7  --exefs7      Specify the 7.x EXEFS Xorpad\n");
    printf("NCSD options:\n");
    printf("  -p  --partition   Specify the partition number to decrypt\n");
    printf("      --extract     Extract the individual partition during decryption\n");
    exit(1);
}

void ParseArgs(int argc, char** argv, optlist* opts, flaglist* flags)
{
    int c;
    while (true) {
        int option_index;
        static struct option long_options[] = {
            { "help",       no_argument,       nullptr, 'h'},
            { "exheader",   required_argument, nullptr, 'e'},
            { "exefs",      required_argument, nullptr, 'x'},
            { "romfs",      required_argument, nullptr, 'r'},
            { "exefs7",     required_argument, nullptr, '7'},

            { "partition",  required_argument, nullptr, 'p'},
            { "extract",    no_argument,       nullptr, 0},
            { 0 },
        };

        c = getopt_long(argc, argv, "he:x:r:7:p:i", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 'e': (*opts)["exheader"]   = optarg; break;
            case 'x': (*opts)["exefs"]      = optarg; break;
            case 'r': (*opts)["romfs"]      = optarg; break;
            case '7': (*opts)["exefs7"]     = optarg; break;
            case 'p': (*opts)["partition"]  = optarg; break;

            case 0: flags->push_back("extract"); break;

            case 'h':
            default:
                ShowHelpInfo();
        }
    }

    if (argc - optind != 1)
        ShowHelpInfo();
}

int main(int argc, char** argv)
{
    optlist opts;
    flaglist flags;
    ParseArgs(argc, argv, &opts, &flags);

    std::string app_file_name;
    std::vector<u8> app_file;
    if (argc - optind == 1) {
        app_file_name = argv[optind++];
        app_file = ReadBinaryFile(app_file_name);
    }

    if (app_file.empty()) {
        printf("ERROR: Input file does not exist!\n");
        return 1;
    }

    if (memcmp(&app_file[NCCH_Header::OFFSET_MAGIC], "NCCH", 4) == 0) {
        NCCH ncch(&app_file[0], app_file.size());
        std::string file_extension;

        if (!DecryptNCCH(opts, &ncch))
            return 1;

        WriteBinaryFile(ReplaceExtension(app_file_name, ncch.GetFileExt()), ncch.GetBuffer(), ncch.GetSize());

    } else if (memcmp(&app_file[NCSD_Header::OFFSET_MAGIC], "NCSD", 4) == 0) {
        NCSD ncsd(&app_file[0], app_file.size());
        std::string file_extension;
        int partition_number;

        if (Found(opts, "partition")) {
            try {
                partition_number = std::stoi(opts.at("partition"));
            } catch (std::invalid_argument) {
                printf("ERROR: Invalid partition number!");
                return 1;
            }
        } else {
            printf("WARNING: Partition number unspecified!\n"
                   "Only decrypting the first partition!\n");
            partition_number = 0;
        }

        NCCH ncch = ncsd.GetNCCH(partition_number);
        if (!DecryptNCCH(opts, &ncch))
            return 1;

        if (Found(flags, "extract")) {
            WriteBinaryFile(ReplaceExtension(app_file_name,
                "part" + std::to_string(partition_number) + "." + ncch.GetFileExt()),
                ncch.GetBuffer(), ncch.GetSize());
            return 0;
        }
        WriteBinaryFile(ReplaceExtension(app_file_name, "cci"), ncsd.GetBuffer(), ncsd.GetSize());

    } else {
        printf("ERROR: Unsupported input file type!");
        return 1;
    }

    return 0;
}
