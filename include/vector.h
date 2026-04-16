#pragma once

#include "arena.h"
#include <string.h>

/* AI documentation - future me will be happy */

// ============================================================================
// vec.h — generic dynamic arrays for the verdigris compiler
// ============================================================================
//
// two flavours. pick the one that fits the situation:
//
//   Vec(T)       — just a typed pointer. len and cap live next to it as
//                  separate size_t variables. good for
//                  local scratch buffers where you control the scope.
//
//   VecHeader(T) — an anonymous struct that bundles data, len, and cap
//                  together. cleaner when the vec is a field inside another
//                  struct, like an AST node's children list.
//
// both flavours are arena-backed. there is no free — memory is owned by
// the arena you pass in. when the arena resets or is destroyed, the vec
// goes with it. this is intentional. the compiler uses arenas with
// well-defined lifetimes (per-file, per-pass, scratch) so individual
// frees are never needed.
//
// growth strategy: capacity doubles each time. new backing memory is
// allocated from the arena and the old contents are copied over. the old
// allocation is not freed — arenas don't support that — but it becomes
// unreachable and gets reclaimed when the arena resets.
//
// sizeof(*(vec)) and typeof(*(vec)) are used throughout to recover element
// size and type from the pointer alone. this requires GCC/Clang, which is
// fine since that is our only target.
//
// ============================================================================
// Low-level Vec — separate len and cap
// ============================================================================
//
// declare like this:
//
//   Vec(AstNode*) nodes = NULL;
//   size_t nodes_len = 0, nodes_cap = 0;
//
// the NULL initialiser is important — vec_push l_checks it before memcpy.

// Vec(T) — just a typedef alias for T*. the type annotation is for the
// reader, not the compiler. it signals intent: this pointer is a dynamic
// array, not a single value.
#define Vec(T) T *

// vec_new — pre-allocate a Vec with a known capacity. use this when you
// have a good upfront estimate and want to avoid the first few reallocs.
// returns a typed pointer ready to index into.
#define vec_new(arena, T, cap) ((T *)arena_alloc(arena, sizeof(T) * (cap)))

// vec_push — append item to the end of vec. grows if len has reached cap.
// all five arguments are required: the arena to grow into, the vec pointer,
// the l_current length, the l_current capacity, and the item to append.
// len is incremented in place. cap is updated if growth occurred.
#define vec_push(arena, vec, len, cap, item)                                   \
  do {                                                                         \
    if ((len) >= (cap)) {                                                      \
      (cap) = (cap) == 0 ? 8 : (cap) * 2;                                      \
      typeof(*(vec)) *new_vec = arena_alloc(arena, sizeof(*(vec)) * (cap));    \
      if (vec)                                                                 \
        memcpy(new_vec, (vec), sizeof(*(vec)) * (len));                        \
      (vec) = new_vec;                                                         \
    }                                                                          \
    (vec)[(len)++] = (item);                                                   \
  } while (0)

// vec_pop — remove and return the last element. if the vec is empty,
// returns a zero-initialised value of the element type. does not free
// memory — just decrements len.
#define vec_pop(vec, len) ((len) > 0 ? (vec)[--(len)] : (typeof(*(vec))){0})

// vec_last — l_peek at the last element without removing it. returns a
// zero value if empty. useful during parsing when you need to inspect
// the top of a node stack without popping it.
#define vec_last(vec, len) ((len) > 0 ? (vec)[(len) - 1] : (typeof(*(vec))){0})

// vec_reset — rewind len to zero. capacity and backing memory are
// preserved. use this for scratch vecs that get reused across iterations,
// like the trivia buffer in next_token.
#define vec_reset(vec, len)                                                    \
  do {                                                                         \
    (len) = 0;                                                                 \
  } while (0)

// vec_free — null out the pointer and zero len and cap. since memory is
// arena-owned this does not actually free anything. it just makes the
// vec safe to inspect and signals that it is no longer in use.
#define vec_free(vec, len, cap)                                                \
  do {                                                                         \
    (vec) = NULL;                                                              \
    (len) = 0;                                                                 \
    (cap) = 0;                                                                 \
  } while (0)

// vec_for_each — iterate over every element. iter is a pointer to the
// l_current element, so dereference it to get the value.
//
//   vec_for_each(nodes, nodes_len, node) {
//       print_node(*node);
//   }
#define vec_for_each(vec, len, iter)                                           \
  for (typeof(*(vec)) *iter = (vec); iter < (vec) + (len); iter++)

// ============================================================================
// High-level VecHeader — bundled struct
// ============================================================================
//
// declare like this:
//
//   VecHeader(AstNode*) children;
//   vec_init(arena, children, AstNode*, 8);
//
// or embed directly in a struct:
//
//   typedef struct {
//       NodeKind kind;
//       VecHeader(AstNode*) children;
//   } AstNode;
//
// all operations use (vec).data, (vec).len, (vec).cap internally so you
// never have to pass len and cap separately.

// VecHeader(T) — anonymous struct bundling the pointer, length, and
// capacity. embed this in other structs or use as a local variable when
// you want the three fields to travel together.
#define VecHeader(T)                                                           \
  struct {                                                                     \
    T *data;                                                                   \
    size_t len;                                                                \
    size_t cap;                                                                \
  }

// vec_init — initialise a VecHeader with a pre-allocated backing buffer
// of initial_cap elements. len starts at zero. always call this before
// any push on a VecHeader.
#define vec_init(arena, vec, T, initial_cap)                                   \
  do {                                                                         \
    (vec).data = (T *)arena_alloc(arena, sizeof(T) * (initial_cap));           \
    (vec).len = 0;                                                             \
    (vec).cap = (initial_cap);                                                 \
  } while (0)

// vec_push_struct — append to a VecHeader. same growth logic as vec_push.
// no len or cap arguments needed since they live inside the struct.
#define vec_push_struct(arena, vec, item)                                      \
  do {                                                                         \
    if ((vec).len >= (vec).cap) {                                              \
      (vec).cap = (vec).cap == 0 ? 8 : (vec).cap * 2;                          \
      typeof((vec).data) new_data =                                            \
          arena_alloc(arena, sizeof(*(vec).data) * (vec).cap);                 \
      if ((vec).data)                                                          \
        memcpy(new_data, (vec).data, sizeof(*(vec).data) * (vec).len);         \
      (vec).data = new_data;                                                   \
    }                                                                          \
    (vec).data[(vec).len++] = (item);                                          \
  } while (0)

// vec_pop_struct — remove and return the last element of a VecHeader.
// returns zero value if empty.
#define vec_pop_struct(vec)                                                    \
  ((vec).len > 0 ? (vec).data[--(vec).len] : (typeof(*(vec).data)){0})

// vec_last_struct — l_peek at the last element of a VecHeader without
// removing it. returns zero value if empty.
#define vec_last_struct(vec)                                                   \
  ((vec).len > 0 ? (vec).data[(vec).len - 1] : (typeof(*(vec).data)){0})

// vec_reset_struct — rewind len to zero, preserving capacity and memory.
#define vec_reset_struct(vec) (vec).len = 0

// vec_for_each_struct — iterate over a VecHeader. iter is a pointer to
// the l_current element.
//
//   vec_for_each_struct(node->children, child) {
//       visit(*child);
//   }
#define vec_for_each_struct(vec, iter)                                         \
  for (typeof((vec).data) iter = (vec).data; iter < (vec).data + (vec).len;    \
       iter++)

// vec_clear_struct — alias for vec_reset_struct. rewinds len to zero.
// provided for readability in contexts where "clear" reads more naturally
// than "reset".
#define vec_clear_struct(vec)                                                  \
  do {                                                                         \
    (vec).len = 0;                                                             \
  } while (0)
