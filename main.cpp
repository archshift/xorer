#include <string>
#include <cstdlib>
#include <vector>
#include <map>
#include <iostream>

#ifdef _MSC_VER
// Required define for MSVC compatibility
#define __STDC__ 1
#endif

#include "getopt/getopt.h"

#include "file_io.h"
#include "common_types.h"
#include "ncch.h"
#include "ncsd.h"

typedef std::map<std::string, std::string> optlist;
typedef std::vector<std::string> flaglist;

bool DecryptCXI(const optlist& args, NCCH* ncch)
{
    if (!Found(args, "exefs") && !Found(args, "exefs7")) {
        std::cerr << "ERROR: 7.x EXEFS XORPads must be accompanied by normal EXEFS XORPads!\n";
        return false;
    }

    if (!Found(args, "exheader")) {
        std::cerr << "ERROR: The input file type requires an exheader xorpad!\n";
        return false;
    }

    if (!ncch->DecryptExheader(ReadBinaryFile(args.at("exheader"))))
        return false;

    if (!Found(args, "exefs")) {
        std::cerr << "ERROR: CXIs without EXEFSs not yet supported!\n";
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
            std::cerr << "ERROR: The input file type requires a ROMFS xorpad!\n";
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
        std::cerr << "ERROR: CFAs with EXEFSs not yet supported!\n";
        return false;
    }

    if (ncch->HasRomFS()) {
        if (!Found(args, "romfs")) {
            std::cerr << "ERROR: The input file type requires a ROMFS xorpad!\n";
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
    std::cerr << "xorer: Apply XORPads to encrypted 3DS files\n";
    std::cerr << "Usage: xorer <file> [-p num] [-e xorpad] [-x xorpad] [-r xorpad] [-7 xorpad]\n";
    std::cerr << "       xorer --dumb <file> <xorpad>\n";
    std::cerr << "  -h  --help        Display this help information\n";
    std::cerr << "      --dumb        XOR the first argument with the second argument\n";
    std::cerr << "NCCH or NCSD options:\n";
    std::cerr << "  -e  --exheader    Specify the Exheader XORPad\n";
    std::cerr << "  -x  --exefs       Specify the (normal) EXEFS Xorpad\n";
    std::cerr << "  -r  --romfs       Specify the ROMFS Xorpad\n";
    std::cerr << "  -7  --exefs7      Specify the 7.x EXEFS Xorpad\n";
    std::cerr << "NCSD options:\n";
    std::cerr << "  -p  --partition   Specify the partition number to decrypt\n";
    std::cerr << "      --extract     Extract the individual partition during decryption\n";
    exit(1);
}

void ParseArgs(int argc, char** argv, optlist* opts, flaglist* flags)
{
    int c;
    while (true) {
        int option_index;
        static struct option long_options[] = {
            { "help",       no_argument,       nullptr, 'h'},
            { "dumb",       no_argument,       nullptr, 1},

            { "exheader",   required_argument, nullptr, 'e'},
            { "exefs",      required_argument, nullptr, 'x'},
            { "romfs",      required_argument, nullptr, 'r'},
            { "exefs7",     required_argument, nullptr, '7'},

            { "partition",  required_argument, nullptr, 'p'},
            { "extract",    no_argument,       nullptr, 0},
            { 0 },
        };

        c = getopt_long(argc, argv, "he:x:r:7:p:", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 'e': (*opts)["exheader"]   = optarg; break;
            case 'x': (*opts)["exefs"]      = optarg; break;
            case 'r': (*opts)["romfs"]      = optarg; break;
            case '7': (*opts)["exefs7"]     = optarg; break;
            case 'p': (*opts)["partition"]  = optarg; break;

            case 0: flags->push_back("extract"); break;
            case 1: flags->push_back("dumb"); break;

            case 'h':
            default:
                ShowHelpInfo();
        }
    }
}

int main(int argc, char** argv)
{
    optlist opts;
    flaglist flags;
    ParseArgs(argc, argv, &opts, &flags);

    std::string app_file_name;
    std::vector<u8> app_file;
    if (Found(flags, "dumb") && argc - optind == 2) {
        app_file_name = argv[optind++];
        app_file = ReadBinaryFile(app_file_name);
        std::vector<u8> xorpad = ReadBinaryFile(argv[optind++]);

        if (app_file.empty()) {
            std::cerr << "ERROR: Input file does not exist!\n";
            return 1;
        } else if (xorpad.empty()) {
            std::cerr << "ERROR: Xorpad file does not exist!\n";
            return 1;
        }

        XOR(&app_file[0], &xorpad[0], app_file.size());
        WriteBinaryFile(app_file_name + ".decrypted", &app_file[0], app_file.size());
        return 0;
        
    } else if (argc - optind == 1) {
        app_file_name = argv[optind++];
        app_file = ReadBinaryFile(app_file_name);
    } else {
        ShowHelpInfo();
    }

    if (app_file.empty()) {
        std::cerr << "ERROR: Input file does not exist!\n";
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
        unsigned int partition_number;

        if (Found(opts, "partition")) {
            try {
                partition_number = str_to_uint(opts.at("partition"));
            } catch (std::invalid_argument) {
                std::cerr << "ERROR: Invalid partition number!";
                return 1;
            }
        } else {
            std::cerr << "WARNING: Partition number unspecified!\n"
                   "Only decrypting the first partition!\n";
            partition_number = 0;
        }

        NCCH ncch = ncsd.GetNCCH(partition_number);
        if (!DecryptNCCH(opts, &ncch))
            return 1;

        if (Found(flags, "extract")) {
            WriteBinaryFile(ReplaceExtension(app_file_name,
                "part" + uint_to_str(partition_number) + "." + ncch.GetFileExt()),
                ncch.GetBuffer(), ncch.GetSize());
            return 0;
        }
        WriteBinaryFile(ReplaceExtension(app_file_name, "cci"), ncsd.GetBuffer(), ncsd.GetSize());

    } else {
        std::cerr << "ERROR: Unsupported input file type!";
        return 1;
    }

    return 0;
}
