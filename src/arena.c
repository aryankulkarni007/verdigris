#include "../include/arena.h"
#include <stdio.h>

/// user must call malloc on buffer and pass the buffer into this function
void arena_init(Arena *arena, void *buffer, size_t cap) {
  arena->start = arena->pos = (void *)buffer;
  arena->end = (char *)buffer + cap;
}

/// rounds the pos to the nearest multiple of 8 for alignment padding
/// optimisation
static uintptr_t arena_align(uintptr_t pos, uintptr_t align) {
  return (pos + (align - 1)) & ~(align - 1);
}

/// returns pointer to start of newly allocated memory
void *arena_allocate(Arena *arena, size_t size) {
  if (size == 0) {
    fprintf(stderr, "error: arena allocate :: size <= 0\n");
    return NULL;
  }

  void *current = NULL;
  uintptr_t aligned = arena_align((size_t)arena->pos, sizeof(void *));

  if (aligned + size <= (char *)arena->end) {
    arena->pos = (void *)aligned;
    current = (void *)arena->pos;
    arena->pos = (char *)arena->pos + size;
  } else {
    fprintf(stderr, "error: arena allocate :: pos + size > capacity\n");
    return NULL;
  }

  return current;
}

/// user's responsibility to free buffer after arena reset
void arena_reset(Arena *arena) { arena->pos = (void *)arena->start; }
