#include "../include/arena.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void parse_and_print_file(const char *path) {
  FILE *file = fopen(path, "rb");
  if (!file) {
    fprintf(stderr, "error: cannot open file '%s'\n", path);
    exit(1);
  }

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  rewind(file);

  char *buffer = malloc(file_size + 1);
  fread(buffer, 1, file_size, file);
  buffer[file_size] = '\0';
  fclose(file);

  Source src = {.buffer = buffer, .file_size = file_size};

  Arena token_arena;
  Token *stream = malloc(file_size * sizeof(Token));
  arena_init(&token_arena, stream, file_size * sizeof(Token));

  Lexer lexer;
  lexer_new(&lexer, src);
  lex(&token_arena, &lexer);

  size_t token_count = 0;
  while (stream[token_count].ttype != TOKEN_EOF) {
    token_count++;
  }

  for (size_t i = 0; i < token_count; i++) {
    token_print(&stream[i]);
  }

  Arena ast_arena;
  size_t ast_size = file_size * 32;
  void *ast_buffer = malloc(ast_size);
  arena_init(&ast_arena, ast_buffer, ast_size);

  Parser parser;
  parser_new(&parser, stream, token_count);
  Module *mod = parse(&ast_arena, &parser);

  printf("\n--- AST for %s ---\n", path);
  ast_print_module(mod);
  printf("\n");

  arena_reset(&token_arena);
  arena_reset(&ast_arena);
  free(stream);
  free(ast_buffer);
  free(buffer);
}

void run_parser_tests(void) {
  printf("\nParser Tests\n");
  printf("==================================\n");

  // Test with a simple Koz file
  parse_and_print_file("text/test_basic.koz");

  printf("==================================\n");
  printf("Parser Tests Complete\n");
}
