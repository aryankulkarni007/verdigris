#include "../include/intern.h"
#include <stdio.h>
#include <string.h>

/* immense AI help - first time hashmap implementation */
#define INITIAL_CAPACITY 256

InternTable intern_init(Arena *arena) {
  InternTable it = {0};

  it.capacity = INITIAL_CAPACITY;
  it.count = 0;
  it.string_arena = arena;

  // entry array
  it.entries = (InternID *)arena_alloc(arena, sizeof(InternID) * it.capacity);
  // zero initialize - key == NULL means empty slot
  memset(it.entries, 0, sizeof(InternID) * it.capacity);
  return it;
}

// grow the hash table when load factor exceeds 0.75
static void intern_grow(InternTable *it) {
  size_t old_capacity = it->capacity;
  InternID *old_entries = it->entries;

  // double capacity
  it->capacity *= 2;
  it->entries = (InternID *)arena_alloc(it->string_arena,
                                        sizeof(InternID) * it->capacity);
  memset(it->entries, 0, sizeof(InternID) * it->capacity);

  // Reinsert all entries
  for (size_t i = 0; i < old_capacity; i++) {
    InternID *entry = &old_entries[i];
    if (entry->key != NULL) {
      size_t index = entry->hash & (it->capacity - 1);

      // linear probe to find empty slot
      while (it->entries[index].key != NULL) {
        index = (index + 1) & (it->capacity - 1);
      }
      it->entries[index] = *entry; // copy entries
    }
  }
}

InternID intern_string(InternTable *it, const char *str, size_t len) {
  uint64_t hash = fnv1a(str, len);
  size_t index = hash & (it->capacity - 1);

  // linear probe
  while (it->entries[index].key != NULL) {
    // if string already exists
    if (it->entries[index].hash == hash && it->entries[index].len == len &&
        memcmp(it->entries[index].key, str, len) == 0) {
      return it->entries[index];
    }
    index = (index + 1) & (it->capacity - 1);
  }
  // check load factor and grow if needed
  if (it->count >= (it->capacity * 3) / 4) { // 0.75 load factor
    intern_grow(it);
    // recompute index after growth
    index = hash & (it->capacity - 1);
    while (it->entries[index].key != NULL) {
      index = (index + 1) & (it->capacity - 1);
    }
  }

  // copy string to arena
  char *copy = (char *)arena_alloc(it->string_arena, len + 1);
  memcpy(copy, str, len);
  copy[len] = '\0';

  // insert new entry
  it->entries[index].key = copy;
  it->entries[index].len = len;
  it->entries[index].hash = hash;
  it->count++;

  return it->entries[index];
}

const char *intern_lookup(InternTable *it, InternID id) {
  (void)it;
  return id.key;
}

bool intern_eq(InternID a, InternID b) {
  return a.key == b.key; // pointer equality since strings are interned
}

InternID intern_cstr(InternTable *it, const char *str) {
  return intern_string(it, str, strlen(str));
}

// info dump
void intern_dump(InternTable *it) {
  printf("=== Intern Table ===\n");
  printf("Capacity: %zu\n", it->capacity);
  printf("Count: %zu\n", it->count);
  printf("Load factor: %.2f%%\n", (float)it->count / it->capacity * 100);
  printf("\nEntries:\n");

  size_t max_probe = 0;
  size_t total_probe = 0;
  size_t used_slots = 0;

  for (size_t i = 0; i < it->capacity; i++) {
    if (it->entries[i].key != NULL) {
      used_slots++;

      // calculate probe distance (how far from ideal slot)
      size_t ideal = it->entries[i].hash & (it->capacity - 1);
      size_t probe_distance = (i - ideal) & (it->capacity - 1);
      total_probe += probe_distance;
      if (probe_distance > max_probe)
        max_probe = probe_distance;

      printf("[%04zu] ", i);
      printf("hash: 0x%016llx ", (unsigned long long)it->entries[i].hash);
      printf("probe: %2zu ", probe_distance);
      printf("len: %3zu ", it->entries[i].len);
      printf("\"%.50s\"", it->entries[i].key);
      if (it->entries[i].len > 50)
        printf("...");
      printf("\n");
    }
  }

  printf("\n=== Statistics ===\n");
  printf("Used slots: %zu\n", used_slots);
  printf("Max probe distance: %zu\n", max_probe);
  printf("Avg probe distance: %.2f\n",
         used_slots ? (float)total_probe / used_slots : 0);
}
