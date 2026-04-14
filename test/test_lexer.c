// test_lexer.c
#include "../include/arena.h"
#include "../include/intern.h"
#include "../include/lexer.h"
#include "../include/token.h"
#include "../include/trivia.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* WARN: AI WRITTEN TESTS */

#define TEST(name) void test_##name(void)
#define RUN_TEST(name)                                                         \
  do {                                                                         \
    printf("  Running %s... ", #name);                                         \
    test_##name();                                                             \
    printf("✓\n");                                                             \
  } while (0)

static void dump_tokens(TStream ts) {
  for (size_t i = 0; i < ts.len; i++) {
    Token *t = &ts.data[i];
    printf("Token %zu: type=%s, span=[%zu..%zu]\n", i, tk_name(t->type),
           t->span.start, t->span.end);
  }
}

// Helper to lex a string
static TStream lex_string(const char *source) {
  Arena token_arena = arena_init(1 << 20);
  Arena trivia_arena = arena_init(1 << 20);
  Arena string_arena = arena_init(1 << 20);
  InternTable intern = intern_init(&string_arena);

  Source src = {
      .file_path = "<test>",
      .file_size = strlen(source),
      .buffer = source,
  };

  Lexer lexer;
  lexer_init(&lexer, &token_arena, &string_arena, &trivia_arena, &src, &intern);
  return lex(&lexer);
}

// Helper to l_check token type
static void expect_token(Token *t, TK_T expected_type) {
  if (t->type != expected_type) {
    fprintf(stderr, "\n  Expected token type %d, got %d\n", expected_type,
            t->type);
    fprintf(stderr, "  At line %d, span [%zu..%zu]\n", t->line, t->span.start,
            t->span.end);
    assert(0);
  }
}

// Helper to l_check token count
static void expect_token_count(TStream stream, size_t expected) {
  if (stream.len != expected) {
    fprintf(stderr, "\n  Expected %zu tokens, got %zu\n", expected, stream.len);
    assert(0);
  }
}

// ============================================================================
// Basic Tokens
// ============================================================================

TEST(basic_identifiers) {
  TStream ts = lex_string("let mut x y42 _hidden");
  expect_token_count(ts, 6); // 5 idents + EOF

  expect_token(&ts.data[0], TK_IDENT); // let
  expect_token(&ts.data[1], TK_IDENT); // mut
  expect_token(&ts.data[2], TK_IDENT); // x
  expect_token(&ts.data[3], TK_IDENT); // y42
  expect_token(&ts.data[4], TK_IDENT); // _hidden
  expect_token(&ts.data[5], TK_EOF);
}

TEST(integer_literals) {
  TStream ts = lex_string("0 42 1000 007");
  dump_tokens(ts); // DEBUG
  expect_token_count(ts, 5);

  expect_token(&ts.data[0], TK_INT);
  expect_token(&ts.data[1], TK_INT);
  expect_token(&ts.data[2], TK_INT);
  expect_token(&ts.data[3], TK_INT);
  expect_token(&ts.data[4], TK_EOF);
}

TEST(float_literals) {
  TStream ts = lex_string("0.0 3.14 1.5 1.0");
  expect_token_count(ts, 5);

  expect_token(&ts.data[0], TK_FLOAT);
  expect_token(&ts.data[1], TK_FLOAT);
  expect_token(&ts.data[2], TK_FLOAT);
  expect_token(&ts.data[3], TK_FLOAT);
  expect_token(&ts.data[4], TK_EOF);
}

TEST(string_literals) {
  TStream ts = lex_string("\"hello\" \"world\" \"\" \"escaped\\\"quote\"");
  dump_tokens(ts); // DEBUG
  expect_token_count(ts, 5);

  expect_token(&ts.data[0], TK_STRING);
  expect_token(&ts.data[1], TK_STRING);
  expect_token(&ts.data[2], TK_STRING);
  expect_token(&ts.data[3], TK_STRING);
  expect_token(&ts.data[4], TK_EOF);
}

// ============================================================================
// Operators
// ============================================================================

TEST(arithmetic_operators) {
  TStream ts = lex_string("+ - * / %");
  expect_token_count(ts, 6);

  expect_token(&ts.data[0], TK_PLUS);
  expect_token(&ts.data[1], TK_MINUS);
  expect_token(&ts.data[2], TK_STAR);
  expect_token(&ts.data[3], TK_SLASH);
  expect_token(&ts.data[4], TK_MODULO);
  expect_token(&ts.data[5], TK_EOF);
}

TEST(compound_assignment) {
  TStream ts = lex_string("+= -= *= /= %=");
  dump_tokens(ts); // DEBUG
  expect_token_count(ts, 6);

  expect_token(&ts.data[0], TK_PLUSEQ);
  expect_token(&ts.data[1], TK_MINUSEQ);
  expect_token(&ts.data[2], TK_STAREQ);
  expect_token(&ts.data[3], TK_SLASHEQ);
  expect_token(&ts.data[4], TK_MODULOEQ);
  expect_token(&ts.data[5], TK_EOF);
}

TEST(comparison_operators) {
  TStream ts = lex_string("== != < > <= >=");
  expect_token_count(ts, 7);

  expect_token(&ts.data[0], TK_EQ);
  expect_token(&ts.data[1], TK_NEQ);
  expect_token(&ts.data[2], TK_LT);
  expect_token(&ts.data[3], TK_GT);
  expect_token(&ts.data[4], TK_LTEQ);
  expect_token(&ts.data[5], TK_GTEQ);
  expect_token(&ts.data[6], TK_EOF);
}

TEST(logical_operators) {
  TStream ts = lex_string("&& || !");
  dump_tokens(ts); // DEBUG
  expect_token_count(ts, 4);

  expect_token(&ts.data[0], TK_AND);
  expect_token(&ts.data[1], TK_OR);
  expect_token(&ts.data[2], TK_BANG);
  expect_token(&ts.data[3], TK_EOF);
}

TEST(special_operators) {
  TStream ts = lex_string("-> => :: .. ..= >>");
  expect_token_count(ts, 7);

  expect_token(&ts.data[0], TK_ARROW);
  expect_token(&ts.data[1], TK_FATARROW);
  expect_token(&ts.data[2], TK_CCOL);
  expect_token(&ts.data[3], TK_DDOT);
  expect_token(&ts.data[4], TK_DDOTEQ);
  expect_token(&ts.data[5], TK_PIPELINE);
  expect_token(&ts.data[6], TK_EOF);
}

// ============================================================================
// Punctuation
// ============================================================================

TEST(delimiters) {
  TStream ts = lex_string("() {} [] ; , . : & | ? @ _");
  dump_tokens(ts); // DEBUG
  expect_token_count(ts, 16);

  expect_token(&ts.data[0], TK_OPAREN);
  expect_token(&ts.data[1], TK_CPAREN);
  expect_token(&ts.data[2], TK_OBRACE);
  expect_token(&ts.data[3], TK_CBRACE);
  expect_token(&ts.data[4], TK_OBRACK);
  expect_token(&ts.data[5], TK_CBRACK);
  expect_token(&ts.data[6], TK_SEMI);
  expect_token(&ts.data[7], TK_COMMA);
  expect_token(&ts.data[8], TK_DOT);
  expect_token(&ts.data[9], TK_COL);
  expect_token(&ts.data[10], TK_AMP);
  expect_token(&ts.data[11], TK_PIPE);
  expect_token(&ts.data[12], TK_QUESTION);
  expect_token(&ts.data[13], TK_AT);
  expect_token(&ts.data[14], TK_UND);
}

// ============================================================================
// Comments and Trivia
// ============================================================================

TEST(single_line_comment) {
  TStream ts = lex_string("-- this is a comment\nlet x = 1;");
  expect_token_count(ts, 6); // let, x, =, 1, ;, EOF

  // Check trivia attached to 'let'
  expect_token(&ts.data[0], TK_IDENT);
  assert(ts.data[0].leading_count > 0);
  assert(ts.data[0].leading[0].type == TV_COMMENT);
}

TEST(block_comment) {
  TStream ts = lex_string("-* block comment *-\nlet x = 1;");
  dump_tokens(ts); // DEBUG
  expect_token_count(ts, 6);

  expect_token(&ts.data[0], TK_IDENT);
  assert(ts.data[0].leading_count > 0);
  assert(ts.data[0].leading[0].type == TV_BLOCKC);
}

TEST(doc_comment) {
  TStream ts = lex_string("--- doc comment\nlet x = 1;");
  expect_token_count(ts, 6);

  expect_token(&ts.data[0], TK_IDENT);
  assert(ts.data[0].leading_count > 0);
  assert(ts.data[0].leading[0].type == TV_DOCC);
}

TEST(mixed_trivia) {
  TStream ts = lex_string("  \t\n-- comment\n-* block *-\nlet x = 1;");
  dump_tokens(ts); // DEBUG
  expect_token_count(ts, 6);

  expect_token(&ts.data[0], TK_IDENT);
  assert(ts.data[0].leading_count >= 5); // space, tab, newline, comment, block
}

// ============================================================================
// Error Cases
// ============================================================================

TEST(unterminated_string) {
  TStream ts = lex_string("\"unterminated");
  expect_token_count(ts, 2);

  expect_token(&ts.data[0], TK_ILLEGAL);
  expect_token(&ts.data[1], TK_EOF);
}

TEST(unterminated_block_comment) {
  TStream ts = lex_string("-* unterminated");
  dump_tokens(ts); // DEBUG
  expect_token_count(ts, 2);

  expect_token(&ts.data[0], TK_ERROR);
  expect_token(&ts.data[1], TK_EOF);
}

// ============================================================================
// Complex Expressions
// ============================================================================

TEST(function_declaration) {
  TStream ts = lex_string("add(int a, int b) -> int { a + b }");

  // add ( int a , int b ) -> int { a + b }
  // 0   1 2   3 4 5   6 7 8  9   10 11 12 13 14
  expect_token_count(ts, 16);

  expect_token(&ts.data[0], TK_IDENT); // add
  expect_token(&ts.data[1], TK_OPAREN);
  expect_token(&ts.data[2], TK_IDENT); // int
  expect_token(&ts.data[3], TK_IDENT); // a
  expect_token(&ts.data[4], TK_COMMA);
  expect_token(&ts.data[5], TK_IDENT); // int
  expect_token(&ts.data[6], TK_IDENT); // b
  expect_token(&ts.data[7], TK_CPAREN);
  expect_token(&ts.data[8], TK_ARROW);
  expect_token(&ts.data[9], TK_IDENT); // int
  expect_token(&ts.data[10], TK_OBRACE);
  expect_token(&ts.data[11], TK_IDENT); // a
  expect_token(&ts.data[12], TK_PLUS);
  expect_token(&ts.data[13], TK_IDENT); // b
  expect_token(&ts.data[14], TK_CBRACE);
}

TEST(generic_enum) {
  TStream ts = lex_string("enum Option[T] { Some(T), None }");
  expect_token_count(ts, 14);

  expect_token(&ts.data[0], TK_IDENT); // enum
  expect_token(&ts.data[1], TK_IDENT); // Option
  expect_token(&ts.data[2], TK_OBRACK);
  expect_token(&ts.data[3], TK_IDENT); // T
  expect_token(&ts.data[4], TK_CBRACK);
  expect_token(&ts.data[5], TK_OBRACE);
  expect_token(&ts.data[6], TK_IDENT); // Some
  expect_token(&ts.data[7], TK_OPAREN);
  expect_token(&ts.data[8], TK_IDENT); // T
  expect_token(&ts.data[9], TK_CPAREN);
  expect_token(&ts.data[10], TK_COMMA);
  expect_token(&ts.data[11], TK_IDENT); // None
  expect_token(&ts.data[12], TK_CBRACE);
  expect_token(&ts.data[13], TK_EOF);
}

TEST(pattern_match) {
  TStream ts = lex_string("match n { 0 => 1, _ => 0 }");
  expect_token_count(ts, 12);

  expect_token(&ts.data[0], TK_IDENT);    // match
  expect_token(&ts.data[1], TK_IDENT);    // n
  expect_token(&ts.data[2], TK_OBRACE);   // {
  expect_token(&ts.data[3], TK_INT);      // 0
  expect_token(&ts.data[4], TK_FATARROW); // =>
  expect_token(&ts.data[5], TK_INT);      // 1
  expect_token(&ts.data[6], TK_COMMA);    // ,
  expect_token(&ts.data[7], TK_UND);      // _
  expect_token(&ts.data[8], TK_FATARROW); // =>
  expect_token(&ts.data[9], TK_INT);      // 0
  expect_token(&ts.data[10], TK_CBRACE);  // }
  expect_token(&ts.data[11], TK_EOF);     // EOF
}

TEST(error_handling) {
  TStream ts = lex_string("file = open(\"data.txt\")!;");
  expect_token_count(ts, 9);

  expect_token(&ts.data[0], TK_IDENT);  // file
  expect_token(&ts.data[1], TK_ASSIGN); // =
  expect_token(&ts.data[2], TK_IDENT);  // open
  expect_token(&ts.data[3], TK_OPAREN); // (
  expect_token(&ts.data[4], TK_STRING); // "data.txt"
  expect_token(&ts.data[5], TK_CPAREN); // )
  expect_token(&ts.data[6], TK_BANG);   // !
  expect_token(&ts.data[7], TK_SEMI);   // ;
  expect_token(&ts.data[8], TK_EOF);    // EOF
}

TEST(pipeline) {
  TStream ts = lex_string("data >> filter(.active) >> map(.name)");
  expect_token_count(ts, 14);

  expect_token(&ts.data[0], TK_IDENT);    // data
  expect_token(&ts.data[1], TK_PIPELINE); // >>
  expect_token(&ts.data[2], TK_IDENT);    // filter
  expect_token(&ts.data[3], TK_OPAREN);   // (
  expect_token(&ts.data[4], TK_DOT);      // .
  expect_token(&ts.data[5], TK_IDENT);    // active
  expect_token(&ts.data[6], TK_CPAREN);   // )
  expect_token(&ts.data[7], TK_PIPELINE); // >>
  expect_token(&ts.data[8], TK_IDENT);    // map
  expect_token(&ts.data[9], TK_OPAREN);   // (
  expect_token(&ts.data[10], TK_DOT);     // .
  expect_token(&ts.data[11], TK_IDENT);   // name
  expect_token(&ts.data[12], TK_CPAREN);  // )
  expect_token(&ts.data[13], TK_EOF);     // EOF
}

// ============================================================================
// Test Runner
// ============================================================================

int main(void) {
  printf("Running lexer tests...\n\n");

  printf("Basic Tokens:\n");
  RUN_TEST(basic_identifiers);
  RUN_TEST(integer_literals);
  RUN_TEST(float_literals);
  RUN_TEST(string_literals);

  printf("\nOperators:\n");
  RUN_TEST(arithmetic_operators);
  RUN_TEST(compound_assignment);
  RUN_TEST(comparison_operators);
  RUN_TEST(logical_operators);
  RUN_TEST(special_operators);

  printf("\nPunctuation:\n");
  RUN_TEST(delimiters);

  printf("\nComments and Trivia:\n");
  RUN_TEST(single_line_comment);
  RUN_TEST(block_comment);
  RUN_TEST(doc_comment);
  RUN_TEST(mixed_trivia);

  printf("\nError Cases:\n");
  RUN_TEST(unterminated_string);
  RUN_TEST(unterminated_block_comment);

  printf("\nComplex Expressions:\n");
  RUN_TEST(function_declaration);
  RUN_TEST(generic_enum);
  RUN_TEST(pattern_match);
  RUN_TEST(error_handling);
  RUN_TEST(pipeline);

  printf("\nAll tests passed!\n");
  return 0;
}
