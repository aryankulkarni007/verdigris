#pragma once

#include "arena.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
  const char *key; // pointer into string arena
  size_t len;      // length of string
  uint64_t hash;   // cached hash
} InternID;

typedef struct {
  InternID *entries;
  size_t capacity; // must be power of two
  size_t count;
  Arena *string_arena;
} InternTable;
// cap being a power of two allows for indexing with & instead of %
// size_t index = hash & (map->capacity - 1);

static inline uint64_t fnv1a(const char *str, size_t len) {
  uint64_t hash = 14695981039346656037ULL;
  for (size_t i = 0; i < len; i++) {
    hash ^= (uint8_t)str[i];
    hash *= 1099511628211ULL;
  }
  return hash;
}

InternTable intern_init(Arena *arena);
InternID intern_string(InternTable *it, const char *str, size_t len);
const char *intern_lookup(InternTable *it, InternID id);
bool intern_eq(InternID a, InternID b);

InternID intern_cstr(InternTable *it, const char *str);
void intern_dump(InternTable *it);
