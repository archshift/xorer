#pragma once

#include <cstdlib>
#include <vector>
#include <map>
#include <algorithm>

#include "common_types.h"

#define INSERT_PADDING_HELPER1(x, y) x ## y
#define INSERT_PADDING_HELPER2(x, y) INSERT_PADDING_HELPER1(x, y)
#define INSERT_PADDING(bytes) u8 INSERT_PADDING_HELPER2(__padding, __LINE__)[(bytes)]

void XOR(u8* target_buf, const u8* xorpad, const size_t size);

bool CompareHash(const u8* data_buf, const size_t data_size, const u8* hash_buf);

template <typename AT, typename T>
bool Found(const std::vector<AT>& vec, T item) {
    return std::find(vec.begin(), vec.end(), item) != vec.end();
}

template <typename MT1, typename MT2, typename T>
bool Found(const std::map<MT1, MT2>& map, T item) {
    return map.find(item) != map.end();
}
