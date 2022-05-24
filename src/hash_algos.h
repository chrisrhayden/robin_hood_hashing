#ifndef HASH_ALGOS
#define HASH_ALGOS

#include <inttypes.h>

uint64_t data_hash64(const void *key, uint64_t len);

uint64_t integer_hash64(uint64_t x);

#endif
