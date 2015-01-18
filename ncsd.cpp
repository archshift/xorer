#include "ncsd.h"

NCSD_Partition_Table NCSD_Header::GetPartitionTable() {
    NCSD_Partition_Table table {{{0}}};
    memcpy(table.partitions, (NCSD_Partition*)&header[NCSD_Header::OFFSET_PARTITION_TABLE], 0x40);
    for (int i = 0; i < 8; ++i) {
        table.partitions[i].offset *= 0x200;
        table.partitions[i].size *= 0x200;
    }
    return table;
}

NCCH NCSD::GetNCCH(size_t part_num) {
    NCSD_Partition partition = header.GetPartitionTable().partitions[part_num];
    return NCCH(buffer + partition.offset, partition.size);
};