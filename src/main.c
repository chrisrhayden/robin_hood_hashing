#include <stdio.h>
#include <stdlib.h>

#include "hash_algos.h"
#include "robin_hood.h"

uint64_t hash(void *key) { return integer_hash64(*(uint64_t *)key); }

bool comp(void *key_1, void *key_2) {
  return *(uint64_t *)key_1 == *(uint64_t *)key_2;
}

int main() {
  Map *map = create_map(hash, comp);

  for (size_t i = 0; i < 20000; i++) {
    uint64_t *key = malloc(sizeof(uint64_t));
    *key = i;

    uint64_t *value = malloc(sizeof(uint64_t));
    *value = i * 2;

    insert_value(map, key, value);
  }

  uint64_t *key = malloc(sizeof(uint64_t));

  *key = 100;

  printf("Finding key\n");
  Item *item = lookup_key(map, key);

  if (item == NULL) {
    printf("%lu dose not exists in hash map\n", *key);
  } else {

    printf("key: %lu value: %lu\n", *(uint64_t *)item->key,
           *(uint64_t *)item->value);
  }

  int64_t count = 0;
  for (uint64_t i = 100; i < 1100; i++) {
    count++;
    *key = i;
    printf("deleting key: %lu\n", *key);

    delete_item(map, key);
  }

  printf("deleted count: %lu\n", count);

  IterMap *iter = create_iter_map(map);

  item = NULL;

  count = 0;
  uint64_t big_vlaue = 0;
  for_each(iter, item) {
    count++;

    // printf("key: %lu value: %lu\n", *(uint64_t *)item->key, *(uint64_t
    // *)item->value);

    if (big_vlaue < *(uint64_t *)item->key) {
      big_vlaue = *(uint64_t *)item->key;
    }
  }

  printf("count: %lu big key: %lu\n", count, big_vlaue);

  return 0;
}
