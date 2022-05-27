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
typedef void (*drop_func_t)(void *key, void *value);

typedef struct {
  Item **buckets;
  uint64_t bucket_len;
  uint64_t bucket_load;
  hash_func_t hash_func;
  drop_func_t drop_func;
} Map;

Map *create_map(hash_func_t hash_func, drop_func_t drop_func);
void drop_map(Map *map);

bool insert_value(Map *map, void *key, void *value);

bool delete_item(Map *map, void *key_to_find, void **key_to_fill,
                 void **value_to_fill);

Item *lookup_key(Map *map, void *key);

typedef struct {
  Item **buckets;
  uint64_t current_pos;
  uint64_t bucket_len;
} IterMap;

IterMap *create_iter_map(Map *map);
void drop_iter_map(IterMap *iter_map);
Item *next_item(IterMap *iter_map);

// this is actually nice
#define for_each(_iter_map, _item)                                             \
  _iter_map->current_pos = 0;                                                  \
  for (_item = next_item(_iter_map); _item != NULL;                            \
       _item = next_item(_iter_map))

void find_avrage_probe(Map *map);

#endif
