#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/arena.h"
#include "../include/lexer.h"
#include "../include/main.h"
#include "../include/vector.h"
#include "parser.h"
#include "pretty.h"

#define error(x)                                                               \
  do {                                                                         \
    fprintf(stderr, "%s\n", x);                                                \
    return (Source){.file_size = 0, .buffer = NULL};                           \
  } while (0)

void print_version() {
  printf("Verdigris Compiler v0.1.0\n");
  return;
}

void print_usage(char *exec) {
  fprintf(stderr, "usage: %s <file>\n", exec);
  return;
}

Source handle_file(char *path) {
  FILE *file = fopen(path, "rb");
  if (!file) {
    error("fopen error");
  }

  if (fseek(file, 0, SEEK_END) == -1) {
    error("fseek error");
  }

  size_t file_size = ftell(file);
  rewind(file);

  char *buffer = malloc(file_size + 1);
  if (!buffer) {
    error("buffer allocation error");
  }

  long result = fread(buffer, 1, file_size, file);
  if (result != (long)file_size) {
    error("fread error");
  }

  buffer[file_size] = '\0';
  fclose(file);
  return (Source){.file_size = file_size, .buffer = buffer, .file_path = path};
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  if (strcmp(argv[1], "-v") == 0) {
    print_version();
    return 0;
  }

  Source src = handle_file(argv[1]);
  // printf("%s", src.buffer);

  Arena token_arena = arena_init(1LL << 30);
  Arena trivia_arena = arena_init(1LL << 30);
  Arena string_arena = arena_init(1LL << 30);
  Arena diag_arena = arena_init(1LL << 30);
  Arena cst_arena = arena_init(1LL << 30);

  Lexer lexer;
  InternTable table = intern_init(&string_arena);
  lexer_init(&lexer, &token_arena, &string_arena, &trivia_arena, &src, &table);
  TStream stream = lex(&lexer);

  // for debugging the string hashmap
  // intern_dump(&table);

  // super cool macro
  // vec_for_each_struct(stream, t) print_token(t, src.buffer);

  DiagBag diags = diag_init(&diag_arena);
  Parser parser;
  parser_init(&parser, stream, &cst_arena, &diags, &table, src.file_path,
              src.buffer);

  cst_d *module = parse(&parser);
  if (!diag_ok(&diags)) {
    diag_print_all(&diags, src.buffer, stderr);
    return 1;
  }

  cst_pretty_init(stderr, &table);
  print_decl(module);
  putc('\n', stderr);

  arena_destroy(&token_arena);
  arena_destroy(&trivia_arena);
  arena_destroy(&string_arena);
  arena_destroy(&diag_arena);
  arena_destroy(&cst_arena);
  return 0;
}
