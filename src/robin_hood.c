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

Map *_create_map(hash_func_t hash, uint64_t size) {
  Map *map = malloc(sizeof(Map));
  map->buckets = (Item **)malloc(sizeof(Item *) * size);
  map->bucket_len = size;
  map->bucket_load = 0;

  for (size_t i = 0; i < size; i++) {
    map->buckets[i] = NULL;
  }

  map->hash_func = hash;

  return map;
}

Map *create_map(hash_func_t hash) {
  return _create_map(hash, STARTING_SIZE);
}

Item *create_item(void *key, void *value, uint64_t hash, uint64_t probe) {
  Item *item = (Item *)malloc(sizeof(Item));

  item->key = key;
  item->value = value;
  item->probe = probe;
  item->hash = hash;

  return item;
}

bool _insert_value(Item **buckets, uint64_t bucket_len, hash_func_t hash_func,
                   void *key, void *value) {

  uint64_t hash = hash_func(key);
  // uint64_t pos = hash & (bucket_len - 1);
  uint64_t pos = hash % bucket_len;

  uint64_t current_pos = pos;

  uint64_t probe = 0;
  Item *item = NULL;

  bool success = true;
  bool finding_bucket = true;

  while (finding_bucket) {

    item = buckets[current_pos];

    if (item == NULL) {
      Item *new_item = create_item(key, value, hash, probe);

      if (new_item == NULL) {
        success = false;
      }

      buckets[current_pos] = new_item;

      finding_bucket = false;
    } else {
      // we found a "rich" item so we will swap it with the poor item
      if (probe > item->probe) {
        swap(&key, &item->key);
        swap(&value, &item->value);

        uint64_t temp_hash = item->hash;
        item->hash = hash;
        hash = temp_hash;

        uint64_t temp_probe = item->probe;
        item->probe = probe;
        probe = temp_probe;
      }

      current_pos = (current_pos + 1) % bucket_len;
      probe++;
    }
  }

  return success;
}

bool rehash_map(Map *map) {
  uint64_t new_len = map->bucket_len * GROWTH_FACTOR;

  Item **new_buckets = (Item **)malloc(sizeof(Item *) * new_len);

  if (new_buckets == NULL) {
    return false;
  }

  bool success = true;

  Item *item = NULL;
  for (uint64_t i = 0; i < map->bucket_len && success; i++) {
    item = map->buckets[i];

    if (item != NULL) {
      success = _insert_value(new_buckets, new_len, map->hash_func, item->key,
                              item->value);
    }
  }

  free(map->buckets);
  map->buckets = new_buckets;
  map->bucket_len = new_len;

  return true;
}

bool insert_value(Map *map, void *key, void *value) {
  if (map->bucket_load + 1 == floor(map->bucket_len * MAX_LOAD_FACTOR)) {
    if (rehash_map(map) == false) {
      return false;
    }
  }

  if (_insert_value(map->buckets, map->bucket_len, map->hash_func, key,
                    value)) {
    map->bucket_load++;

    return true;
  }

  return false;
}

void backward_shift(Map *map, uint64_t pos) {
  uint64_t current_pos = pos;

  Item *item = NULL;

  bool move_backward = true;

  while (move_backward) {
    current_pos = (current_pos + 1) % map->bucket_len;
    item = map->buckets[current_pos];

    if (item == NULL || item->probe == 0) {
      move_backward = false;
    } else {
      item->probe--;
      map->buckets[current_pos - 1] = item;
      map->buckets[current_pos] = NULL;
    }
  }
}

Item *delete_item(Map *map, void *key) {
  uint64_t hash = map->hash_func(key);
  uint64_t pos = hash % map->bucket_len;
  uint64_t probe = 0;

  Item *item = NULL;
  uint64_t current_pos = 0;

  bool finding_bucket = true;

  while (finding_bucket) {
    current_pos = (pos + probe) % map->bucket_len;

    item = map->buckets[current_pos];

    if (item == NULL || probe > item->probe) {
      item = NULL;
      finding_bucket = false;
    } else if (item->hash == hash) {
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
  uint64_t pos = hash % map->bucket_len;
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
      if (item->hash == hash) {
        finding_bucket = false;

        // if the item probe is larger the we would have found the key
        // we are looking for by now
      } else if (probe > item->probe) {
        item = NULL;
        finding_bucket = false;
      }

      probe++;
    }
  }

  return item;
}

IterMap *create_iter_map(Map *map) {
  IterMap *iter_map = (IterMap *)malloc(sizeof(IterMap));

  if (iter_map == NULL) {
    return NULL;
  }

  iter_map->current_pos = 0;
  iter_map->buckets = map->buckets;
  iter_map->bucket_len = map->bucket_len;

  return iter_map;
}

Item *next_item(IterMap *iter_map) {
  Item *item = NULL;

  bool looking = true;

  while (looking && iter_map->current_pos <= iter_map->bucket_len) {
    item = iter_map->buckets[iter_map->current_pos];

    if (item != NULL) {
      looking = false;
    }

    iter_map->current_pos++;
  }

  return item;
}
