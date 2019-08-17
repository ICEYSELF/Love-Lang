#include "tree_dump.h"

#include "mstring.h"
#include "y.tab.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

typedef void (*tree_node_printer)(ast_node_base_t*);
typedef void (*tree_link_printer)(uint16_t, int,
                                  ast_node_base_t*, strhdl_t);

static const char* ans_strs[] = {
  #define ANS_DECLARE(KEY) #KEY ,
  #include "ast_sema_info.h"
  ""
};

static const char *ank_strs[] = {
  #define ANK_DECLARE(KEY) #KEY ,
  #include "ast_node_kind.h"
  ""
};

static void plast_tlp(uint16_t parent_id, int nth_child,
                      ast_node_base_t *node, strhdl_t extra_info) {
  printf("%u[%d] -- (%s) -> NODE %u,",
         (unsigned)parent_id, nth_child, get_string(extra_info),
         node->node_uid);
}

static void plast_tnp(ast_node_base_t *node) {
  printf(" ANK = %s, ANS = %s",
         ank_strs[node->node_kind],
         ans_strs[node->node_sema_info]);
  if (strstr(ank_strs[node->node_kind], "WDATA")) {
    if (node->node_sema_info == ANS_IDREF
        || node->node_sema_info == ANS_FUNCTION
        || node->node_sema_info == ANS_STR) {
      printf(", SDATA = %s",
             get_string(((ast_node_wdata_base_t*)node)
                          ->value.svalue));
    }
    else {
      printf(", IDATA = %" PRId64,
             ((ast_node_wdata_base_t*)node)->value.ivalue);
    }
  }
  putchar('\n');
}

static void pldot_tlp(uint16_t parent_id, int nth_child,
                      ast_node_base_t *node, strhdl_t extra_info) {
  (void)extra_info;
  printf("  node%u -- node%u [label=\"%dth child\"];\n",
         (unsigned)parent_id, node->node_uid, nth_child);
}

static void pldot_tnp(ast_node_base_t *node) {
  static char buffer[256] = "";
  if (strstr(ank_strs[node->node_kind], "WDATA")) {
    jjvalue_t value = ((ast_node_wdata_base_t*)node)->value;
    switch (node->node_sema_info) {
    case ANS_IDREF:
      sprintf(buffer, "Identifier %s", get_string(value.svalue));
      break;
    case ANS_FUNCTION:
      sprintf(buffer, "Function %s", get_string(value.svalue));
      break;
    case ANS_STR:
      sprintf(buffer, "'%s'", get_string(value.svalue));
      break;
    case ANS_INTVAL:
      sprintf(buffer, "%" PRId64, value.ivalue); break;
    case ANS_FLOATVAL:
      sprintf(buffer, "%f", value.fvalue); break;
    case ANS_BOOLVAL:
      sprintf(buffer, "%s", value.bvalue ? "True" : "False"); break;
    case ANS_BINEXPR: {
      const char *s = "";
      switch (value.ivalue) {
      case TK_ESYM_PLUS: s = "+"; break;
      case TK_ESYM_MINUS: s = "-"; break;
      case TK_ESYM_ASTER: s = "*"; break;
      case TK_ESYM_SLASH: s = "/"; break;
      case TK_ESYM_PERCENT: s = "%"; break;
      case TK_ESYM_AMPAMP: s = "&&"; break;
      case TK_ESYM_PIPEPIPE: s = "||"; break;
      case TK_ESYM_LT: s = "<"; break;
      case TK_ESYM_GT: s = ">"; break;
      case TK_ESYM_LEQ: s = "<="; break;
      case TK_ESYM_GEQ: s = ">="; break;
      case TK_ESYM_EQ: s = "="; break;
      case TK_ESYM_EQEQ: s = "=="; break;
      case TK_ESYM_NEQ: s = "!="; break;
      }
      sprintf(buffer, "%s", s);
      break;
    default:
      sprintf(buffer, "(Not printable)");
    }
    }
  }
  else {
    switch (node->node_sema_info) {
    case ANS_FUNCTIONS: sprintf(buffer, "Func List"); break;
    case ANS_IDS: sprintf(buffer, "Id List"); break;
    case ANS_STATEMENTS: sprintf(buffer, "Statements"); break;
    case ANS_FUNCTION_BODY: sprintf(buffer, "Function Body"); break;
    case ANS_IF: sprintf(buffer, "If"); break;
    case ANS_WHILE: sprintf(buffer, "While"); break;
    case ANS_RETURN: sprintf(buffer, "Return"); break;
    case ANS_FUNC_CALL: sprintf(buffer, "Call"); break;
    case ANS_SEMI_SEP_LIST: sprintf(buffer, "Expr List"); break;
    case ANS_NULL: sprintf(buffer, "Empty Statement"); break;
    default: sprintf(buffer, "(Not printable)");
    }
  }
  printf("  node%d [shape=box label=\"%s\"];\n",
         node->node_uid, buffer);
}

static void
tree_dump_impl(ast_node_base_t *node, uint16_t parent_id,
               int nth_child, strhdl_t extra_info,
               tree_node_printer tnp, tree_link_printer tlp) {
#ifdef _Static_assert
  _Static_assert(ANS_COMMENCE_ == 0, "Incorrect ANS order!");
  _Static_assert(ANK_COMMENCE_ == 0, "Incorrect ANK order!");
#endif

  tlp(parent_id, nth_child, node, extra_info);
  tnp(node);

  switch (node->node_kind) {
  default:
  case ANK_LEAF:
  case ANK_LEAF_WDATA:
    break;

  case ANK_SINGLE_CHILD:
    tree_dump_impl(((ast_schild_t*)node)->child, node->node_uid, 0,
                   create_string(""), tnp, tlp);
    break;

  case ANK_SINGLE_CHILD_WDATA:
    tree_dump_impl(((ast_schild_wdata_t*)node)->child,
                   node->node_uid, 0,
                   create_string(""), tnp, tlp);
    break;

  case ANK_DUAL_CHILD:
    for (int i = 0; i < 2; i++) {
      tree_dump_impl(((ast_dchild_t*)node)->children[i],
                     node->node_uid, i,
                     create_string(""), tnp, tlp);
    }
    break;

  case ANK_DUAL_CHILD_WDATA:
    for (int i = 0; i < 2; i++) {
      tree_dump_impl(((ast_dchild_wdata_t*)node)->children[i],
                     node->node_uid, i,
                     create_string(""), tnp, tlp);
    }
    break;

  case ANK_TRIPLE_CHILD:
    for (int i = 0; i < 3; i++) {
      tree_dump_impl(((ast_tchild_t*)node)->children[i],
                     node->node_uid, i,
                     create_string(""), tnp, tlp);
    }
    break;

  case ANK_TRIPLE_CHILD_WDATA:
    for (int i = 0; i < 3; i++) {
      tree_dump_impl(((ast_tchild_wdata_t*)node)->children[i],
                     node->node_uid, i,
                     create_string(""), tnp, tlp);
    }
    break;

  case ANK_LIST: {
    ast_list_t *list = (ast_list_t*)node;
    int i = 0;
    for (iter_t it = list_begin(&(list->list));
         !iter_eq(it, list_end(&(list->list)));
         it = iter_next(it)) {
      tree_dump_impl((ast_node_base_t*)iter_deref(it),
                     node->node_uid, i,
                     create_string(""), tnp, tlp);
      i++;
    }
    break;
  }
  }
}

void tree_dump_plast(ast_node_base_t *root) {
  tree_dump_impl(root, 0, 0, create_string(""), plast_tnp, plast_tlp);
}

void tree_dump_pldot(ast_node_base_t *root) {
  printf("graph plast {\n");
  tree_dump_impl(root, 0, 0, create_string(""), pldot_tnp, pldot_tlp);
  printf("node0 [label=\"Translation Unit\"];");
  printf("}\n");
}
