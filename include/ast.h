#ifndef AST_H
#define AST_H

#include "clist.h"
#include "jjvalue.h"
#include "util.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  #define ANK_DECLARE(KEY) KEY,
  #include "ast_node_kind.h"
  ANK_INVALID
} ast_node_kind_t;

typedef enum {
  #define ANS_DECLARE(KEY) KEY,
  #include "ast_sema_info.h"
  ANS_INVALID
} ast_node_sema_t;

#define AST_NODE_COMMON \
  uint8_t node_kind; \
  uint8_t node_sema_info; \
  uint16_t node_uid; \
  uint16_t row; \
  uint16_t col;

typedef struct {
  AST_NODE_COMMON
} ast_node_base_t;

typedef struct {
  AST_NODE_COMMON
  jjvalue_t value;
} ast_node_wdata_base_t;

#if __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(ast_node_base_t) == 8,
               "Unexpected size difference");
_Static_assert(sizeof(ast_node_wdata_base_t) == 16,
               "Unexpected size difference");
#endif

typedef struct {
  AST_NODE_COMMON
} ast_leaf_t;

typedef struct {
  AST_NODE_COMMON
  jjvalue_t value;
} ast_leaf_wdata_t;

typedef struct {
  AST_NODE_COMMON
  ast_node_base_t *child;
} ast_schild_t;

typedef struct {
  AST_NODE_COMMON
  jjvalue_t value;
  ast_node_base_t *child;
} ast_schild_wdata_t;

typedef struct {
  AST_NODE_COMMON
  ast_node_base_t *children[2];
} ast_dchild_t;

typedef struct {
  AST_NODE_COMMON
  jjvalue_t value;
  ast_node_base_t *children[2];
} ast_dchild_wdata_t;

typedef struct {
  AST_NODE_COMMON
  ast_node_base_t *children[3];
} ast_tchild_t;

typedef struct {
  AST_NODE_COMMON
  jjvalue_t value;
  ast_node_base_t *children[3];
} ast_tchild_wdata_t;

typedef struct {
  AST_NODE_COMMON
  list_t TP(ast_node_base_t*) list;
} ast_list_t;

ast_node_base_t *leaf(ast_node_sema_t sema_info);

ast_node_base_t *leaf_wdata(ast_node_sema_t sema_info, jjvalue_t data);

ast_node_base_t *node1(ast_node_sema_t sema_info,
                         ast_node_base_t *child);

ast_node_base_t *node1_wdata(ast_node_sema_t sema_info, 
                               jjvalue_t data,
                               ast_node_base_t *child);

ast_node_base_t *node2(ast_node_sema_t sema_info,
                       ast_node_base_t *lchild,
                       ast_node_base_t *rchild);

ast_node_base_t *node2_wdata(ast_node_sema_t sema_info, 
                               jjvalue_t data,
                               ast_node_base_t *lchild,
                               ast_node_base_t *rchild);

ast_node_base_t *node3(ast_node_sema_t sema_info,
                         ast_node_base_t *child0,
                         ast_node_base_t *child1,
                         ast_node_base_t *child2);

ast_node_base_t *node3_wdata(ast_node_sema_t sema_info,
                               jjvalue_t data,
                               ast_node_base_t *child0,
                               ast_node_base_t *child1,
                               ast_node_base_t *child2);

ast_node_base_t *node_list(ast_node_sema_t sema_info);

void ast_list_prepend(ast_node_base_t *node, ast_node_base_t *data);

void ast_list_append(ast_node_base_t *node, ast_node_base_t *data);

void ast_attach_srcloc(ast_node_base_t *node,
                       uint16_t row, uint16_t col);

void destroy_ast(ast_node_base_t *root);

#ifdef __cplusplus
}
#endif

#endif
