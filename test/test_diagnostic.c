// test/test_diagnostic.c
#include "../include/arena.h"
#include "../include/diagnostic.h"
#include "../include/span.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* WARN: AI WRITTEN TESTS */

int main(void) {
  printf("Running diagnostic tests...\n\n");

  Arena arena = arena_init(1LL << 30);

  // ========================================================================
  // Test 1: Basic error creation
  // ========================================================================
  {
    DiagBag db = diag_init(&arena);

    SourceSpan error_span = {
        .path = "test.ri",
        .start = 43,
        .end = 48,
        .line_start = 3,
        .line_end = 3,
        .col_start = 9,
        .col_end = 14,
    };

    diag_error(&db, error_span, "E0001", "unexpected token 'let'");

    assert(db.has_errors == true);
    assert(db.has_warnings == false);
    assert(db.diagnostics.len == 1);
    assert(db.diagnostics.data[0].level == DIAG_ERROR);
    assert(strcmp(db.diagnostics.data[0].code, "E0001") == 0);
    assert(strcmp(db.diagnostics.data[0].message, "unexpected token 'let'") ==
           0);

    printf("✓ Basic error creation\n");
  }

  // ========================================================================
  // Test 2: Warning creation
  // ========================================================================
  {
    DiagBag db = diag_init(&arena);

    SourceSpan span = {
        .path = "test.ri",
        .start = 10,
        .end = 11,
        .line_start = 2,
        .line_end = 2,
        .col_start = 5,
        .col_end = 6,
    };

    diag_warning(&db, span, "W0001", "unused variable 'x'");

    assert(db.has_errors == false);
    assert(db.has_warnings == true);
    assert(db.diagnostics.len == 1);
    assert(db.diagnostics.data[0].level == DIAG_WARNING);
    assert(strcmp(db.diagnostics.data[0].code, "W0001") == 0);

    printf("✓ Warning creation\n");
  }

  // ========================================================================
  // Test 3: Multiple diagnostics
  // ========================================================================
  {
    DiagBag db = diag_init(&arena);

    SourceSpan span1 = {
        .path = "test.ri", .line_start = 1, .col_start = 1, .col_end = 1};
    SourceSpan span2 = {
        .path = "test.ri", .line_start = 2, .col_start = 1, .col_end = 1};

    diag_error(&db, span1, "E0002", "type mismatch");
    diag_warning(&db, span2, "W0002", "deprecated function");
    diag_note(&db, span1, "expected 'int', found 'string'");
    diag_help(&db, "try casting with 'as'");

    assert(db.has_errors == true);
    assert(db.has_warnings == true);
    assert(db.diagnostics.len == 4);
    assert(db.diagnostics.data[0].level == DIAG_ERROR);
    assert(db.diagnostics.data[1].level == DIAG_WARNING);
    assert(db.diagnostics.data[2].level == DIAG_NOTE);
    assert(db.diagnostics.data[3].level == DIAG_HELP);

    printf("✓ Multiple diagnostics\n");
  }

  // ========================================================================
  // Test 4: diag_ok helper
  // ========================================================================
  {
    DiagBag db = diag_init(&arena);
    SourceSpan span = {.path = "test.ri"};

    assert(diag_ok(&db) == true);

    diag_warning(&db, span, "W0003", "something suspicious");
    assert(diag_ok(&db) == true); // Warnings don't fail diag_ok

    diag_error(&db, span, "E0004", "something broken");
    assert(diag_ok(&db) == false); // Errors do fail

    printf("✓ diag_ok helper\n");
  }

  // ========================================================================
  // Test 5: Pretty printing (visual inspection)
  // ========================================================================
  {
    DiagBag db = diag_init(&arena);

    const char *source = "let x = 42;\n"
                         "let y = \"hello\";\n"
                         "let z = x + y;\n";

    // Error at "x + y" on line 3
    SourceSpan error_span = {
        .path = "test.ri",
        .start = 26, // Points to 'x'
        .end = 31,   // Points to 'y'
        .line_start = 3,
        .line_end = 3,
        .col_start = 9,
        .col_end = 14,
    };

    diag_error(&db, error_span, "E0003", "cannot add 'int' and 'string'");
    diag_help(&db, "convert 'int' to 'string' with .to_string()");

    printf("\n--- Pretty print output (should show colored error) ---\n\n");
    diag_print_all(&db, source, stdout);
    printf("--- End of pretty print output ---\n\n");

    printf("✓ Pretty printing (visual check)\n");
  }

  // ========================================================================
  // Test 6: Formatted messages
  // ========================================================================
  {
    DiagBag db = diag_init(&arena);
    SourceSpan span = {
        .path = "test.ri", .line_start = 5, .col_start = 3, .col_end = 6};

    diag_error(&db, span, "E0005", "expected %d arguments, got %d", 3, 2);

    assert(strcmp(db.diagnostics.data[0].message,
                  "expected 3 arguments, got 2") == 0);

    printf("✓ Formatted messages\n");
  }

  printf("\n✅ All diagnostic tests passed!\n");
  return 0;
}
