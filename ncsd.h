#pragma once

#include <vector>

#include "common_types.h"
#include "ncch.h"

struct NCSD_Partition {
    u32 offset;
    u32 size;
};

struct NCSD_Partition_Table {
    NCSD_Partition partitions[8];
};

class NCSD_Header {
    const u8* header;
public:
    enum : size_t {
        OFFSET_MAGIC = 0x100,
        OFFSET_PARTITION_TABLE = 0x120,
    };

    NCSD_Header(const u8* ncsd_data):
        header(ncsd_data) { }

    NCSD_Partition_Table GetPartitionTable();
};


class NCSD {
    u8* buffer;
    size_t size;
    NCSD_Header header;
public:

    NCSD(u8* buffer, size_t size): buffer(buffer), size(size), header(buffer) {}

    const u8* GetBuffer() { return buffer; }
    const size_t GetSize() { return size; }
    NCCH GetNCCH(size_t part_num);
};