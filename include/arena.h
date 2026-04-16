#pragma once

#include <stddef.h>

#define ARENA_START 1024

typedef struct {
  unsigned char *base; // start of 1GB reservation
  size_t reserved;     // total size
  size_t committed;    // active ram
  size_t used;         // the 'bump' pointer offset
} Arena;

Arena arena_init(size_t reserve_size);
void *arena_alloc(Arena *a, size_t size);
void arena_destroy(Arena *a);
