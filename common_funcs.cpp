#include "common_funcs.h"
#include "common_types.h"

#include <memory>
#include <sstream>

#include "polarssl/sha256.h"

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

unsigned int str_to_uint(const std::string str)
{
    char * end;
    unsigned int val = strtoul(str.c_str(), &end, 0);
    if (*end != '\0')
        throw std::invalid_argument("Invalid string");
    return val;
}

std::string uint_to_str(unsigned int num)
{
    std::ostringstream stream;
    stream << num;
    return stream.str();
}
