#include "../include/parser.h"

// Stmt *parse_stmt(Parser *p, Arena *a) {
//   switch (CURRENT(p).ttype) {
//   case TOKEN_LET:
//     return parse_let_stmt(p, a);
//   case TOKEN_WHILE:
//     return parse_while_stmt(p, a);
//   case TOKEN_FOR:
//     return parse_for_stmt(p, a);
//   case TOKEN_LOOP:
//     return parse_loop_stmt(p, a);
//   case TOKEN_RETURN:
//     return parse_return_stmt(p, a);
//   case TOKEN_BREAK:
//     return parse_break_stmt(p, a);
//   case TOKEN_CONTINUE:
//     return parse_continue_stmt(p, a);
//   case TOKEN_LBRACE:
//     return parse_block_stmt(p, a); // S_BLOCK, different from block expr
//   default: {
//     Expr *e = parse_expr(p, a, PREC_NONE);
//     return ast_stmt_expr(a, CURRENT(p), e);
//   }
//   }
// }
