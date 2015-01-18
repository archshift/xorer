#include "common_funcs.h"
#include "common_types.h"

#include <memory>

#include "polarssl/sha256.h"

#define INSERT_PADDING(size) u8 __FILE__ ## __LINE__ ## _padding[(size)]

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