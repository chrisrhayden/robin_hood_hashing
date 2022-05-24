// robin_hood.h

#ifndef ROBIN_HOOD
#define ROBIN_HOOD

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

#define STARTING_SIZE 1024
#define GROWTH_FACTOR 2
#define MAX_LOAD_FACTOR 0.7

typedef struct {
  void *key;
  void *value;
  uint64_t hash;
  // the probe shouldn't be very big but im keeping it a uint64_t for
  // consistency and to not worry about over flow, though i doubt any probe will
  // be grater the 255
  uint64_t probe;
} Item;

typedef uint64_t (*hash_func_t)(void *key);
typedef bool (*compare_func_t)(void *key_1, void *key_2);

typedef struct {
  Item **buckets;
  uint64_t bucket_len;
  uint64_t bucket_load;
  compare_func_t compare_func;
  hash_func_t hash_func;
} Map;

Map *create_map(hash_func_t hash_func, compare_func_t compare_func);

bool insert_value(Map *map, void *key, void *value);

Item *delete_item(Map *map, void *key);

Item *lookup_key(Map *map, void *key);

typedef struct {
  Item **buckets;
  uint64_t current_pos;
  uint64_t bucket_len;
} IterMap;

IterMap *create_iter_map(Map *map);
Item *next_item(IterMap *iter_map);

#define iter_map(_map, _iter) _iter = create_iter_map(_map)

#define for_each(_iter_map, _item)                                             \
  _iter_map->current_pos = 0;                                                  \
  for (_item = next_item(_iter_map); _item != NULL;                            \
       _item = next_item(_iter_map))

#endif
