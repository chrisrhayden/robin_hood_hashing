// robin_hood.c

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "hash_algos.h"
#include "robin_hood.h"

// swap two pointers
void swap(void **a, void **b) {
  void *temp = *a;
  *a = *b;
  *b = temp;
}

Map *_create_map(hash_func_t hash, compare_func_t comp, uint64_t size) {
  Map *map = malloc(sizeof(Map));
  map->buckets = (Item **)malloc(sizeof(Item *) * size);
  map->bucket_len = size;
  map->bucket_load = 0;

  for (size_t i = 0; i < size; i++) {
    map->buckets[i] = NULL;
  }

  map->hash_func = hash;
  map->compare_func = comp;

  return map;
}

Map *create_map(hash_func_t hash, compare_func_t comp) {
  return _create_map(hash, comp, STARTING_SIZE);
}

Item *create_item(void *key, void *value, uint64_t hash, uint64_t probe) {
  Item *item = (Item *)malloc(sizeof(Item));

  item->key = key;
  item->value = value;
  item->probe = probe;
  item->hash = hash;

  return item;
}

bool rehash_map(Map *map) {
  printf("rehash\n");
  uint64_t new_size = map->bucket_len * GROWTH_FACTOR;

  Map *new_map = _create_map(map->hash_func, map->compare_func, new_size);

  if (new_map == NULL) {
    return false;
  }

  Item *item = NULL;
  for (uint64_t i = 0; i < map->bucket_len; i++) {
    item = map->buckets[i];

    if (item != NULL) {
      insert_value(new_map, item->key, item->value);
    }
  }

  free(map->buckets);
  map->buckets = new_map->buckets;
  map->bucket_len = new_map->bucket_len;

  return true;
}

bool insert_value(Map *map, void *key, void *value) {
  if (map->bucket_load + 1 == floor(map->bucket_len * MAX_LOAD_FACTOR)) {
    if (rehash_map(map) == false) {
      return false;
    }
  }

  uint64_t hash = map->hash_func(key);
  uint64_t pos = hash % map->bucket_len;

  uint64_t probe = 0;

  Item *item = NULL;

  bool finding_bucket = true;

  while (finding_bucket) {
    uint64_t current_pos = (pos + probe) % map->bucket_len;

    item = map->buckets[current_pos];

    if (item == NULL) {
      // TODO: check for null
      map->buckets[current_pos] = create_item(key, value, hash, probe);

      finding_bucket = false;
    } else {
      // we found a "rich" item so we will swap it with the poor item
      if (probe > item->probe) {
        swap(&key, &item->key);
        swap(&value, &item->value);

        uint64_t temp_hash = item->hash;
        item->hash = temp_hash;
        hash = temp_hash;

        uint64_t temp_probe = item->probe;
        item->probe = probe;
        probe = temp_probe;

      } else {
        probe++;
      }
    }
  }

  map->bucket_load++;

  return true;
}

void backward_shift(Map *map, uint64_t pos) {
  uint64_t current_pos = pos;

  Item *item = NULL;

  bool move_backward = true;

  while (move_backward) {
    current_pos++;
    item = map->buckets[current_pos];

    if (item == NULL || item->probe == 0) {
      move_backward = false;
    } else {
      item->probe--;
      map->buckets[current_pos - 1] = item;
      map->buckets[current_pos] = NULL;
      printf("move back key: %lu old probe: %lu new probe: %lu\n",
             *(uint64_t *)item->key, item->probe + 1, item->probe);
    }
  }
}

Item *delete_item(Map *map, void *key) {
  uint64_t hash = map->hash_func(key);
  uint64_t pos = hash & (map->bucket_len - 1);
  uint64_t probe = 0;

  Item *item = NULL;
  uint64_t current_pos = 0;

  bool finding_bucket = true;

  while (finding_bucket) {
    current_pos = pos + probe;
    item = map->buckets[current_pos];

    if (item == NULL) {
      finding_bucket = false;
    } else if (map->compare_func(key, item->key)) {
      finding_bucket = false;
      map->buckets[current_pos] = NULL;

      backward_shift(map, current_pos);
    } else {
      probe++;
    }
  }

  return item;
}

Item *lookup_key(Map *map, void *key) {
  uint64_t hash = map->hash_func(key);
  uint64_t pos = hash & (map->bucket_len - 1);
  uint64_t probe = 0;

  Item *item = NULL;

  uint64_t current_pos = 0;
  bool finding_bucket = true;
  while (finding_bucket) {
    current_pos = pos + probe % map->bucket_len;

    item = map->buckets[current_pos];

    if (item == NULL) {
      finding_bucket = false;
    } else {
      // if this is true then we found the correct bucket
      if (map->compare_func(key, item->key)) {
        finding_bucket = false;

        // if the item probe is larger the we would have found the key we are
        // looking for by now
      } else if (probe > item->probe) {
        item = NULL;
        finding_bucket = false;
      }

      probe++;
    }
  }

  return item;
}
