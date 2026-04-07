#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/arena.h"
#include "../include/lexer.h"
#include "../include/main.h"
#include "../include/parser.h"
#include "../include/token.h"

void print_usage(char *exec);
Source file_handler(char *path);
void run_all_tests(void);

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

  long bytes_read = fread(buffer, 1, file_size, file);
  if (bytes_read != file_size) {
    fprintf(stderr, "error: fread\n");
    exit(1);
  }

  buffer[file_size] = '\0';
  return (Source){.file_size = (size_t)file_size, .buffer = buffer};
}

void run_all_tests(void) {
  run_ast_tests();
  run_parser_tests();
}

int main(int argc, char *argv[]) {
  if (argc > 1 && strcmp(argv[1], "--test") == 0) {
    run_all_tests();
    return 0;
  }

  // normal operation requires a file
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  Source src = file_handler(argv[1]);

  Arena token_arena;
  Token *stream = malloc(src.file_size * sizeof(Token));
  arena_init(&token_arena, stream, src.file_size * sizeof(Token));

  Lexer lexer;
  lexer_new(&lexer, src);
  lex(&token_arena, &lexer);

  size_t token_count = 0;
  while (stream[token_count].ttype != TOKEN_EOF) {
    token_count++;
  }

  // AST arena
  Arena ast_arena;
  size_t ast_size = src.file_size * 32;
  void *ast_buffer = malloc(ast_size);
  arena_init(&ast_arena, ast_buffer, ast_size);

  Parser parser;
  parser_new(&parser, stream, token_count);
  Module *mod = parse(&ast_arena, &parser);
  ast_print_module(mod);

  arena_reset(&token_arena);
  arena_reset(&ast_arena);
  free(stream);
  free(ast_buffer);
  return 0;
}
