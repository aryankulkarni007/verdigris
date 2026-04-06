#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
  char *start;
  char *pos;
  char *end;
} Arena;

// for only tokens for now, so offset doesn't matter
void arena_init(Arena *arena, void *buffer, size_t cap);
void *arena_allocate(Arena *arena, size_t size);
void arena_reset(Arena *arena);

#endif // ARENA_H
