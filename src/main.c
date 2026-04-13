#include <stdio.h>
#include <stdlib.h>

#include "../include/arena.h"
#include "../include/lexer.h"
#include "../include/main.h"

#define ERROR(x)                                                               \
  fprintf(stderr, "%s\n", x);                                                  \
  return (Source) { .file_size = 0, .buffer = NULL }

void print_usage(char *exec) {
  fprintf(stderr, "usage: %s <file>\n", exec);
  return;
}

Source handle_file(char *path) {
  FILE *file = fopen(path, "rb");
  if (!file) {
    ERROR("fopen error");
  }

  if (fseek(file, 0, SEEK_END) == -1) {
    ERROR("fseek error");
  }

  size_t file_size = ftell(file);
  rewind(file);

  char *buffer = malloc(file_size + 1);
  if (!buffer) {
    ERROR("buffer allocation error");
  }

  long result = fread(buffer, 1, file_size, file);
  if (result != (long)file_size) {
    ERROR("fread error");
  }

  buffer[file_size] = '\0';
  return (Source){.file_size = file_size, .buffer = buffer};
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  Source src = handle_file(argv[1]);
  // printf("%s", src.buffer);

  Arena token_arena = arena_init(1LL << 30);
  Arena trivia_arena = arena_init(1LL << 30);
  Arena string_arena = arena_init(1LL << 30);

  Lexer lexer;
  new_lexer(&lexer, &token_arena, &string_arena, &trivia_arena, &src);
  Token *stream = lex(&lexer);

  size_t t_count = 0;
  for (t_count = 0; stream[t_count].type != TK_EOF; ++t_count) {
    print_token(&stream[t_count], src.buffer);
  }
  return 0;
}
