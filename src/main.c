#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/arena.h"
#include "../include/token.h"

typedef struct {
  size_t file_size;
  char *buffer;
} Source;

void print_usage(char *exec);
Source file_handler(char *path);

/// WARNING: remember to free buffer
void print_usage(char *exec) {
  printf("usage: %s <*.koz>", exec);
  return;
}

Source file_handler(char *path) {
  FILE *file = fopen(path, "rb");
  if (!file) {
    fprintf(stderr, "error: fopen\n");
    exit(1);
  }

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  if (file_size < 0) {
    fprintf(stderr, "error: ftell\n");
    exit(1);
  }
  rewind(file);

  char *buffer = malloc(file_size + 1);
  if (!buffer) {
    fprintf(stderr, "error: file buffer malloc\n");
    exit(1);
  }

  size_t bytes_read = fread(buffer, 1, file_size, file);
  if (bytes_read != file_size) {
    fprintf(stderr, "error: fread\n");
    exit(1);
  }

  buffer[file_size] = '\0';
  return (Source){.file_size = (size_t)file_size, .buffer = buffer};
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  Source src = file_handler(argv[1]);
  printf("%s\n", src.buffer);

  Arena arena;
  // using file_size to guess the memory size needed
  Token *stream = malloc(src.file_size * sizeof(Token));
  arena_init(&arena, stream, src.file_size * sizeof(Token));

  Token *new = arena_allocate(&arena, sizeof(Token));
  token_new(new, TOKEN_IDENT, 0, 0, "x");
  token_print(*new);

  arena_reset(&arena);
  free(stream);
  return 0;
}
