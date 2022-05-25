#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hash_algos.h"
#include "robin_hood.h"

bool contains(uint64_t **buckets, uint64_t bucket_len, uint64_t value) {
  bool found = false;
  for (uint64_t i = 0; i < bucket_len && !found; i++) {
    found = *buckets[i] == value;
  }

  return found;
}

uint64_t hash(void *key) {
  return integer_hash64(*(uint64_t *)key);
}

bool add_values(Map *map, uint64_t amount) {
  bool success = true;

  for (uint64_t i = 0; i < amount && success; i++) {
    uint64_t *key = malloc(sizeof(uint64_t));
    *key = i;

    uint64_t *value = malloc(sizeof(uint64_t));
    *value = i * 2;

    success = insert_value(map, key, value);
  }

  return success;
}

bool remove_values(Map *map, uint64_t **items, uint64_t start, uint64_t stop) {
  Item *item = NULL;

  bool found = true;

  uint64_t current = 0;

  for (uint64_t i = start; i <= stop && found; i++) {
    printf("looking for: %lu\n", i);
    item = lookup_key(map, &i);

    if (item == NULL) {
      printf("did not find the item to delete\n");
      found = false;
    }
  }

  for (uint64_t i = start; i <= stop && found; i++) {
    // printf("removing: %lu\n", i);

    item = delete_item(map, &i);

    if (item == NULL) {
      found = false;
    } else {
      items[current] = item->key;
      current++;
    }
  }

  return found;
}

bool map_contains(Map *map, uint64_t **items, uint64_t items_len) {
  Item *item = NULL;
  bool finding = true;
  bool success = false;

  for (uint64_t i = 0; i < items_len && finding; i++) {

    // we reached the end of the item list
    if (items[i] == NULL) {
      success = true;
      finding = false;
    } else {
      printf("looking for %lu\n", *items[i]);
      item = lookup_key(map, items[i]);

      if (item == NULL) {
        finding = false;
      }
    }
  }

  return success;
}

bool run() {
  Map *map = create_map(hash);

  if (add_values(map, 20000) == false) {
    printf("did not add all items\n");
    return false;
  }

  uint64_t **items = (uint64_t **)malloc(sizeof(uint64_t *) * map->bucket_len);

  uint64_t item_value = 100;
  for (uint64_t i = 0; i <= 1000; i++) {
    items[i] = malloc(sizeof(uint64_t));
    *items[i] = item_value;
    item_value++;
  }

  if (map_contains(map, items, map->bucket_len)) {
    printf("found all items\n");
  } else {
    printf("did not find all items\n");
    return false;
  }

  if (remove_values(map, items, 100, 1100) == false) {
    printf("did not remove all items\n");
    return false;
  }

  if (map_contains(map, items, map->bucket_len)) {
    printf("found an item that should have been deleted\n");
    return false;
  } else {
    printf("all items where deleted\n");
  }

  IterMap *iter = create_iter_map(map);
  Item *item = NULL;

  uint64_t count = 0;
  for_each(iter, item) {
    printf("item: %lu\n", *(uint64_t *)item->key);
    count++;
  }

  printf("count: %lu\n", count);

  return true;
}

bool speed_test() {
  clock_t start = clock();

  Map *map = create_map(hash);

  if (add_values(map, 20000) == false) {
    printf("did not add all items\n");
    return false;
  }

  IterMap *iter = create_iter_map(map);
  Item *item = NULL;

  for_each(iter, item) {
    if (lookup_key(map, item->key) == false) {
      printf("did not find item\n");
      break;
    }
  }

  clock_t end = clock();

  printf("Elapsed: %f\n", (double)(end - start) / CLOCKS_PER_SEC);

  return true;
}

int main() {
  if (speed_test()) {
    return 0;
  }

  return 1;
}
