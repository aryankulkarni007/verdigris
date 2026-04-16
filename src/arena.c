#include "../include/arena.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

Arena arena_init(size_t reserve_size) {
  Arena a = {0};
  a.reserved = reserve_size;

  // reserve the address space
  // MAP_ANON - not backed by a file
  // PROT_NONE - trying to read and write here will crash
  a.base = mmap(NULL, a.reserved, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);

  // commit first chunk so we have writing permission
  size_t init_commit = 64 * 1024;
  mprotect(a.base, init_commit, PROT_READ | PROT_WRITE);
  a.committed = init_commit;
  return a;
}

void *arena_alloc(Arena *a, size_t size) {
  size = (size + 7) & ~7;

  if (a->used + size > a->committed) {
    size_t page_size = 64 * 1024;
    size_t needed = a->used + size - a->committed;
    size_t to_commit = ((needed + page_size - 1) / page_size) * page_size;

    if (a->committed + to_commit > a->reserved) {
      fprintf(stderr, "Arena OOM\n");
      abort();
    }

    mprotect((uint8_t *)a->base + a->committed, to_commit,
             PROT_READ | PROT_WRITE);

    a->committed += to_commit;
  }

  void *ptr = (uint8_t *)a->base + a->used;
  a->used += size;
  return ptr;
}
void arena_destroy(Arena *a) {
  // unmap the mmaped memory
  munmap(a->base, a->reserved);
}
