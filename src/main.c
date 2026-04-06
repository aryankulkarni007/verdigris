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
void run_ast_tests(void);

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

int main(int argc, char *argv[]) {
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  if (argc > 1 && strcmp(argv[1], "--test-ast") == 0) {
    // Put your test_ast.c logic into a function called run_ast_tests()
    run_ast_tests();
    return 0;
  }

  Source src = file_handler(argv[1]);
  // printf("%s\n", src.buffer);

  Arena token_arena;
  // using file_size to guess the memory size needed
  Token *stream = malloc(src.file_size * sizeof(Token));
  arena_init(&token_arena, stream, src.file_size * sizeof(Token));

  Lexer lexer;
  lexer_new(&lexer, src);
  lex(&token_arena, &lexer);

  size_t token_count = 0;
  for (token_count = 0; stream[token_count].ttype != TOKEN_EOF; ++token_count) {
    token_print(&stream[token_count]);
  }

  // TODO: comment the above out and uncomment this once we have implemented
  // parsing

  // while (stream[token_count].ttype != TOKEN_EOF) token_count++;

  // new arena for ast
  Arena ast_arena;
  size_t ast_size = src.file_size * 8; // rough guesstimate
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
