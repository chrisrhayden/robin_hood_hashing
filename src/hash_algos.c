// hash_algos.c

#include <inttypes.h>
#include <stdlib.h>

/* this was taken from
 * https://github.com/DavidLeeds/hashmap/
 * blob/137d60b3818c22c79d2be5560150eb2eff981a68/src/hashmap.c#L601
 *
 * Recommended hash function for data keys.
 *
 * This is an implementation of the well-documented Jenkins one-at-a-time
 * hash function. See https://en.wikipedia.org/wiki/Jenkins_hash_function
 */
uint64_t data_hash64(const void *data, uint64_t len) {
  // cast the data to an 8bit pointer so we can iterate over all the data in
  // memory
  //
  // use sizeof() to find the length of an object
  //
  // sizeof(THING) will return the amount of 8bit blocks for THING on most
  // platforms as far as i can tell
  const uint8_t *byte = (const uint8_t *)data;
  uint64_t hash = 0;

  for (size_t i = 0; i < len; ++i) {
    hash += *byte++;
    hash += (hash << 10);
    hash ^= (hash >> 6);
  }

  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);

  return hash;
}

/** integer hash function
 *
 * this is taken from this stackoverflow
 * https://stackoverflow.com/a/12996028
 * and this article i guess
 * https://zimbry.blogspot.com/2011/09/better-bit-mixing-improving-on.html
 */
uint64_t integer_hash64(uint64_t x) {
  x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
  x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
  x = x ^ (x >> 31);
  return x;
}

/* taken from
 * https://github.com/skeeto/hash-prospector
 */
uint32_t integer_hash32(uint32_t x) {
  x ^= x >> 16;
  x *= 0x7feb352d;
  x ^= x >> 15;
  x *= 0x846ca68b;
  x ^= x >> 16;
  return (uint32_t)x;
}
