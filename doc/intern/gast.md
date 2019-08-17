# PiLang GAST documentation

## What is & Why GAST
__GAST__ (__G__eneral __A__bstract __S__yntax __T__ree) is an AST 
library for DSL designers. It allows several kinds of tree nodes to be 
created and some informations to be attached with these created nodes.

## GAST nodes
GAST has the following kinds of nodes:
<lo>
  <li>leaf node               (`ast_leaf_t`)</li>
  <li>leaf node with data     (`ast_leaf_wdata_t`)</li>
  <li>single child node       (`ast_schild_t`)</li>
  <li>single child with data  (`ast_schild_wdata_t`)</li>
  <li>dual child node         (`ast_dchild_t`)</li>
  <li>dual child with data    (`ast_dchild_wdata_t`)</li>
  <li>triple child node       (`ast_tchild_t`)</li>
  <li>triple child with data  (`ast_tchild_wdata_t`)</li>
  <li>list                    (`ast_list_t`)<sup>[1]</sup></li>
</lo>
<sup>[1]</sup> List elements shall only be AST nodes since further
cleanup only knows how to correctly destroy AST nodes.

To create these nodes, here's utility functions:

    // yields ast_leaf_t
    ast_node_base_t *leaf(ast_sema_info_t);

    // yields ast_leaf_wdata_t
    ast_node_base_t *leaf_wdata(ast_sema_info_t, jjvalue_t);

    // yields ast_schild_t
    ast_node_base_t *node1(ast_sema_info_t, ast_node_base_t*);

    // yields ast_schild_wdata_t
    ast_node_base_t *node1(ast_sema_info_t, jjvalue_t,
                           ast_node_base_t*);

    // yields ast_dchild_t
    ast_node_base_t *node2(ast_sema_info_t,
                           ast_node_base_t*, ast_node_base_t*);
    
    // yields ast_dchild_wdata_t
    ast_node_base_t *node2_wdata(ast_sema_info_t, jjvalue_t,

    // yields ast_tchild_t
    ast_node_base_t *node3(ast_sema_info_t, ast_node_base_t*,
                           ast_node_base_t*, ast_node_base_t*);

    // yields ast_tchild_wdata_t
    ast_node_base_t *node3_wdata(ast_sema_info_t, jjvalue_t,
                                 ast_node_base_t*, ast_node_base_t*,
                                 ast_node_base_t*);

    // yields ast_list_t
    ast_node_base_t *node_list(ast_sema_info_t);

What's more, there are two more manipulation functions for `ast_list_t`:

    void ast_list_prepend(ast_node_base_t*, ast_node_base_t*);

    void ast_list_append(ast_node_base_t*, ast_node_base_t*);

And they all have a common base `ast_node_base_t`. As we all know, C
language does not have inheritance. So we use special trick to simulate
inheritance. We made each kind of node start with `AST_NODE_COMMON`:

    #define AST_NODE_COMMON \
      uint8_t node_kind; \
      uint8_t sema_info; \
      uint16_t node_uid; \
      uint16_t row; \
      uint16_t col;

As a result, pointer to any kind of node can be safely converted to 
pointer to `ast_node_base_t`. The current size of `AST_NODE_COMMON` is 8
bytes and suits most CPUs.

For each kind of node, there is a __node kind__ starting with `ANK_`. 
for example, the ID for `ast_leaf_t` is `ANK_LEAF`.

For each node, a unique __UID__ is set. This is used for AST printing 
and debugging.

## Extra source location info
GAST allows extra source location info to be attached with an AST node.
In `AST_NODE_COMMON`, there are two fields `uint16_t row` and 
`uint16_t col`, where you can put in your source location information.
This is quite limited. If you need better machanism, see "Extending
GAST structure".

What's more, one utility function is also provided for attaching source
location info:

    void ast_attach_srcloc(ast_node_base_t*, uint16_t, uint16_t);

## Extra semantics info
GAST allows extra semantics info to be attached with an AST node. In
`AST_NODE_COMMON`, there is a `uint16_t sema_info` where you can put in
your semantics information. To declare a sema info, add a `ANS_DECLARE`
entry into file `ast_sema_info.h`.
Please note that GAST library itself never takes care of handling your
attached semantics information. Rather, it is on your own to correctly
pass them when creating nodes with utility functions.

## Extending GAST structure
To add a new kind of AST node for GAST, for example, you want a kind of
node which have 4 children and 2 pieces of extra data:

    ADT strange_node {
      jjvalue_t data[2];
      ast_node_base_t *children[4];
    }

First, create a struct beginning with `AST_NODE_COMMON`, and add members
according to your requirements:

    typedef struct {
      AST_NODE_COMMON
      jjvalue_t data[2];
      ast_node_base_t *children[4];
    } ast_strange_node_t;

And then add it to `ast.h`. Register your node in `ast_node_kind.h` with 
macro `ANK_DECLARE`

    ANK_DECLARE(ANK_STRANGE)

And it is recommended to add a utility creation function for your new 
node:

    ast_node_base_t *strange_node(ast_sema_info_t sema_info,
                                  jjvalue_t data0, jjvalue_t data1,
                                  ast_node_base_t *child0,
                                  ast_node_base_t *child1,
                                  ast_node_base_t *child2,
                                  ast_node_base_t *child3) {
      // It is recommended to use NEW to allocate memory.
      ast_strange_node_t *ret = NEW(ast_strange_node_t);

      // Setup node kind info and attached sema info
      ret->node_kind = ANK_STRANGE;
      ret->sema_info = sema_info;

      // then initialize other members
      ...

      return (ast_node_base_t*)ret;
    }

Add the function to `ast.c` and add the corresponding declaration to
`ast.h`. Congratulations, that's done!
