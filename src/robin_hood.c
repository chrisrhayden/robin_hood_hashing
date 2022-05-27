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

Map *_create_map(hash_func_t hash, drop_func_t drop, uint64_t size) {
  Map *map = (Map *)malloc(sizeof(Map));

  if (map == NULL) {
    return NULL;
  }

  map->buckets = (Item **)malloc(sizeof(Item *) * size);

  if (map->buckets == NULL) {
    free(map);
    return NULL;
  }

  map->bucket_len = size;
  map->bucket_load = 0;

  for (size_t i = 0; i < size; i++) {
    map->buckets[i] = NULL;
  }

  map->hash_func = hash;
  map->drop_func = drop;

  return map;
}

Map *create_map(hash_func_t hash, drop_func_t drop) {
  return _create_map(hash, drop, STARTING_SIZE);
}

void drop_map(Map *map) {
  Item *item = NULL;

  for (uint64_t i = 0; i < map->bucket_len; i++) {
    item = map->buckets[i];

    if (item != NULL) {
      map->drop_func(item->key, item->value);
      free(item);
    }
  }

  free(map->buckets);
  free(map);
}

Item *create_item(void *key, void *value, uint64_t hash, uint64_t probe) {
  Item *item = (Item *)malloc(sizeof(Item));

  if (item == NULL) {
    return NULL;
  }

  item->key = key;
  item->value = value;
  item->probe = probe;
  item->hash = hash;

  return item;
}

bool _find_bucket() {
  return false;
}

/** insert a value in to the hashmap
 *
 * this will probe for a place in the hashmap and insert/swap when appropriate
 */
bool _insert_value(Item **buckets, uint64_t bucket_len, hash_func_t hash_func,
                   void *key, void *value) {

  // get the hash from the key
  uint64_t hash = hash_func(key);
  // value & (array_len - 1) works like modulo
  uint64_t pos = hash & (bucket_len - 1);

  // set the probe to 0
  uint64_t probe = 0;
  // set the current_pos to the desired position
  uint64_t current_pos = pos;

  // get an item variable for convince
  Item *item = NULL;

  // a flag to return to the user if the insertion was successful
  bool success = true;
  // a flag so we know if we are still looking for a bucket
  bool finding_bucket = true;

  // loop while we need to find a bucket
  while (finding_bucket) {
    // get a potential item form the buckets
    item = buckets[current_pos];

    // if null just make a new item
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

      // increment the current_pos and allow for it to wrap around the array if
      // it would otherwise be out of bounds
      current_pos = (current_pos + 1) & (bucket_len - 1);
      // increment the probe count
      //
      // since we are doing this the end of the loop a swapped probe count
      // should still be incremented as it will be one place farther in the next
      // iteration
      probe++;
    }
  }

  return success;
}

/** rehash the map
 *
 * nothing to it
 */
bool rehash_map(Map *map) {
  // uint64_t new_len = map->bucket_len * GROWTH_FACTOR;
  uint64_t new_len = map->bucket_len * GROWTH_FACTOR;

  Item **new_buckets = (Item **)malloc(sizeof(Item *) * new_len);

  if (new_buckets == NULL) {
    return false;
  }

  for (uint64_t i = 0; i < new_len; i++) {
    new_buckets[i] = NULL;
  }

  bool success = true;

  Item *item = NULL;
  // iterate over the old buckets and add all items to the new bucket
  for (uint64_t i = 0; i < map->bucket_len && success; i++) {
    item = map->buckets[i];

    if (item != NULL) {
      success = _insert_value(new_buckets, new_len, map->hash_func, item->key,
                              item->value);

      // this should be fine as the pointers are being kept and new Item structs
      // are being allocated
      free(item);
    }
  }

  if (success) {
    free(map->buckets);
    map->buckets = new_buckets;
    map->bucket_len = new_len;
  } else {
    free(new_buckets);
  }

  return success;
}

/** insert a key and value into the hashmap
 *
 * this will check if the hashmap should be rehashed in to a bigger hashmap
 */
bool insert_value(Map *map, void *key, void *value) {
  // check if we reached our growth limit
  if (map->bucket_load + 1 == floor(map->bucket_len * MAX_LOAD_FACTOR)) {
    if (rehash_map(map) == false) {
      return false;
    }
  }

  if (_insert_value(map->buckets, map->bucket_len, map->hash_func, key,
                    value)) {
    // only increment the load if we know the key/value was added
    map->bucket_load++;

    return true;
  }

  return false;
}

/** shift all item following a deletion back if applicable
 *
 * this should keep lookups fast and not require a tombstone i guess
 */
void backward_shift(Map *map, uint64_t pos) {
  // the pos where we just deleted from
  uint64_t current_pos = pos;

  Item *item = NULL;

  bool move_backward = true;

  while (move_backward) {
    current_pos = (current_pos + 1) & (map->bucket_len - 1);
    item = map->buckets[current_pos];

    // if no item of the probe count is already 0 then there is nothing to do
    if (item == NULL || item->probe == 0) {
      move_backward = false;
    } else {
      // shift back an item
      item->probe--;
      map->buckets[current_pos - 1] = item;
      map->buckets[current_pos] = NULL;
    }
  }
}

/** remove an item from the map
 *
 * this will return the removed item if found
 */
bool delete_item(Map *map, void *key, void **key_fill, void **value_fill) {
  uint64_t hash = map->hash_func(key);
  uint64_t pos = hash & (map->bucket_len - 1);

  uint64_t probe = 0;
  uint64_t current_pos = pos;

  Item *item = NULL;
  bool finding_bucket = true;

  while (finding_bucket) {
    item = map->buckets[current_pos];

    // if null or the current probe is larger then the item we are checking then
    // the item we are looking for dose not exist
    if (item == NULL || probe > item->probe) {
      item = NULL;
      finding_bucket = false;

    } else if (item->hash == hash) {
      // we found the item we are looking for so remove it and back up all
      // relevant items
      map->buckets[current_pos] = NULL;
      finding_bucket = false;

      backward_shift(map, current_pos);
    }

    current_pos = (current_pos + 1) & (map->bucket_len - 1);
    probe++;
  }

  if (item) {
    *value_fill = item->value;
    *key_fill = item->key;

    free(item);

    return true;
  }

  return false;
}

/** lookup and return an item
 *
 * if we find an item return a pointer to it
 */
Item *lookup_key(Map *map, void *key) {
  uint64_t hash = map->hash_func(key);
  uint64_t pos = hash & (map->bucket_len - 1);

  Item *item = NULL;

  uint64_t probe = 0;
  uint64_t current_pos = pos;

  bool finding_bucket = true;

  while (finding_bucket) {
    item = map->buckets[current_pos];

    // if the item probe is larger the we would have found the key
    // we are looking for by now
    if (item == NULL || probe > item->probe) {
      item = NULL;
      finding_bucket = false;
    } else if (item->hash == hash) {
      // found the correct bucket
      finding_bucket = false;
    }

    probe++;
    current_pos = (current_pos + 1) & (map->bucket_len - 1);
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

void drop_iter_map(IterMap *iter_map) {
  free(iter_map);
}

Item *next_item(IterMap *iter_map) {
  Item *item = NULL;

  bool looking = true;

  while (looking && iter_map->current_pos < iter_map->bucket_len) {
    item = iter_map->buckets[iter_map->current_pos];

    if (item != NULL) {
      looking = false;
    }

    iter_map->current_pos++;
  }

  return item;
}

void find_avrage_probe(Map *map) {
  uint64_t longest = 0;
  uint64_t probe_total = 0;
  uint64_t probe_count = 0;

  Item *item = NULL;

  for (uint64_t i = 0; i < map->bucket_len; i++) {
    item = map->buckets[i];

    if (item) {
      if (item->probe > longest) {
        longest = item->probe;
      }
      probe_total += item->probe;

      probe_count++;
    }
  }

  // uint64_t avg = probe_total / probe_count;
  uint64_t avg = probe_count / probe_total;

  printf("longest probe: %lu probe count: %lu probe total: %lu avg: %lu\n",
         longest, probe_count, probe_total, avg);
}
