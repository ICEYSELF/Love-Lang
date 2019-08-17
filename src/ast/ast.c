#include "ast.h"

#include "mstring.h"
#include "util.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint16_t glob_uid_ = 0;

static uint16_t get_next_uid(void) {
  ++glob_uid_;
  return glob_uid_;
}

ast_node_base_t *leaf(ast_node_sema_t sema_info) {
  ast_leaf_t *ret = NEW(ast_leaf_t);
  ret->node_sema_info = sema_info;
  ret->node_kind = ANK_LEAF;
  ret->node_uid = get_next_uid();
  return (ast_node_base_t*)ret;
}

ast_node_base_t *leaf_wdata(ast_node_sema_t sema_info,
                              jjvalue_t data) {
  ast_leaf_wdata_t *ret = NEW(ast_leaf_wdata_t);
  ret->node_kind = ANK_LEAF_WDATA;
  ret->node_sema_info = sema_info;
  ret->node_uid = get_next_uid();
  ret->value = data;
  return (ast_node_base_t*)ret;
}

ast_node_base_t *node1(ast_node_sema_t sema_info,
                       ast_node_base_t *child) {
  ast_schild_t *ret = NEW(ast_schild_t);
  ret->node_kind = ANK_SINGLE_CHILD;
  ret->node_sema_info = sema_info;
  ret->node_uid = get_next_uid();
  ret->child = child;
  return (ast_node_base_t*)ret;
}

ast_node_base_t *node1_wdata(ast_node_sema_t sema_info,
                               jjvalue_t data,
                               ast_node_base_t *child) {
  ast_schild_wdata_t *ret = NEW(ast_schild_wdata_t);
  ret->node_kind = ANK_SINGLE_CHILD_WDATA;
  ret->node_sema_info = sema_info;
  ret->node_uid = get_next_uid();
  ret->value = data;
  ret->child = child;
  return (ast_node_base_t*)ret;
}

ast_node_base_t *node2(ast_node_sema_t sema_info,
                       ast_node_base_t *lchild,
                       ast_node_base_t *rchild) {
  ast_dchild_t *ret = NEW(ast_dchild_t);
  ret->node_kind = ANK_DUAL_CHILD;
  ret->node_sema_info = sema_info;
  ret->node_uid = get_next_uid();
  ret->children[0] = lchild;
  ret->children[1] = rchild;
  return (ast_node_base_t*)ret;
}

ast_node_base_t *node2_wdata(ast_node_sema_t sema_info,
                               jjvalue_t data,
                               ast_node_base_t *lchild,
                               ast_node_base_t *rchild) {
  ast_dchild_wdata_t *ret = NEW(ast_dchild_wdata_t);
  ret->node_kind = ANK_DUAL_CHILD_WDATA;
  ret->node_sema_info = sema_info;
  ret->node_uid = get_next_uid();
  ret->value = data;
  ret->children[0] = lchild;
  ret->children[1] = rchild;
  return (ast_node_base_t*)ret;
}

ast_node_base_t *node3(ast_node_sema_t sema_info,
                       ast_node_base_t *child0,
                       ast_node_base_t *child1,
                       ast_node_base_t *child2) {
  ast_tchild_t *ret = NEW(ast_tchild_t);
  ret->node_kind = ANK_TRIPLE_CHILD;
  ret->node_sema_info = sema_info;
  ret->node_uid = get_next_uid();
  ret->children[0] = child0;
  ret->children[1] = child1;
  ret->children[2] = child2;
  return (ast_node_base_t*)ret;
}

ast_node_base_t *node3_wdata(ast_node_sema_t sema_info,
                               jjvalue_t data,
                               ast_node_base_t *child0,
                               ast_node_base_t *child1,
                               ast_node_base_t *child2) {
  ast_tchild_wdata_t *ret = NEW(ast_tchild_wdata_t);
  ret->node_kind = ANK_TRIPLE_CHILD_WDATA;
  ret->node_sema_info = sema_info;
  ret->node_uid = get_next_uid();
  ret->value = data;
  ret->children[0] = child0;
  ret->children[1] = child1;
  ret->children[2] = child2;
  return (ast_node_base_t*)ret;
}

ast_node_base_t *node_list(ast_node_sema_t sema_info) {
  ast_list_t *ret = NEW(ast_list_t);
  ret->node_kind = ANK_LIST;
  ret->node_sema_info = sema_info;
  ret->node_uid = get_next_uid();
  create_list(&(ret->list), malloc, free);
  return (ast_node_base_t*)ret;
}

void ast_list_prepend(ast_node_base_t *node,
                      ast_node_base_t *data) {
  assert(node->node_kind == ANK_LIST);
  ast_list_t *list = (ast_list_t*)node;
  list_push_front(&(list->list), data);
}

void ast_list_append(ast_node_base_t *node,
                     ast_node_base_t *data) {
  assert(node->node_kind == ANK_LIST);
  ast_list_t *list = (ast_list_t*)node;
  list_push_back(&(list->list), data);
}

void ast_attach_srcloc(ast_node_base_t *node,
                       uint16_t row, uint16_t col) {
  node->row = row;
  node->col = col;
}

void destroy_ast(ast_node_base_t *root) {
  switch (root->node_kind) {
  case ANK_LEAF:
  case ANK_LEAF_WDATA:
    break;

  case ANK_SINGLE_CHILD: {
    ast_schild_t *schild = (ast_schild_t*)root;
    destroy_ast(schild->child);
    break;
  }

  case ANK_DUAL_CHILD: {
    ast_dchild_t *dchild = (ast_dchild_t*)root;
    destroy_ast(dchild->children[0]);
    destroy_ast(dchild->children[1]);
    break;
  }

  case ANK_TRIPLE_CHILD: {
    ast_tchild_t *tchild = (ast_tchild_t*)root;
    destroy_ast(tchild->children[0]);
    destroy_ast(tchild->children[1]);
    destroy_ast(tchild->children[2]);
    break;
  }

  case ANK_SINGLE_CHILD_WDATA: {
    ast_schild_wdata_t *schild = (ast_schild_wdata_t*)root;
    destroy_ast(schild->child);
    break;
  }

  case ANK_DUAL_CHILD_WDATA: {
    ast_dchild_wdata_t *dchild = (ast_dchild_wdata_t*)root;
    destroy_ast(dchild->children[0]);
    destroy_ast(dchild->children[1]);
    break;
  }

  case ANK_TRIPLE_CHILD_WDATA: {
    ast_tchild_wdata_t *tchild = (ast_tchild_wdata_t*)root;
    destroy_ast(tchild->children[0]);
    destroy_ast(tchild->children[1]);
    destroy_ast(tchild->children[2]);
    break;
  }

  case ANK_LIST: {
    ast_list_t *list = (ast_list_t*)root;
    for (iter_t it = list_begin(&(list->list));
         !iter_eq(it, list_end(&(list->list)));
         it = iter_next(it)) {
      destroy_ast((ast_node_base_t*)iter_deref(it));
    }
    destroy_list(&(list->list));
  }
  }

  free(root);
}
