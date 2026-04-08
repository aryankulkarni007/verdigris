#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct {
  char *start;
  char *pos;
  char *end;
} Arena;

// for only tokens for now, so offset doesn't matter
void arena_init(Arena *arena, void *buffer, size_t cap);
void *arena_allocate(Arena *arena, size_t size);
void arena_reset(Arena *arena);

// neccessary to fix string corruptions issues:
// we model the token as a char\[256\] but whenever we need to use it as a
// char*, we end up doing &char\[0\], which aliases the stack frame. we need to
// copy to avoid it
static inline char *arena_strdup(Arena *a, const char *s) {
  size_t len = strlen(s) + 1;
  char *copy = (char *)arena_allocate(a, len);
  memcpy(copy, s, len);
  return copy;
}

#endif // ARENA_H
