# PiLang AST nodes
This is a documentation introducing PiLang AST node kinds.

## PiLang node kinds

* _[shell]_: this production does not have concrete AST node but simply
forward others ASTs
* _[wrapper]_: this production has an AST which simply wraps other
AST nodes
* _[list]_: this production has an AST list node
* _[entity]_: this production does have concrete AST node

### `program` node _[shell]_
* __Production__: `program: functions` <br/>

### `functions` node _[list]_
* __Production__: `functions function | <empty>` 
* __ANK__: `ANK_LIST` 
* __ANS__: `ANS_FUNCTIONS` 
* __Children__: `function`s 

### `function` node _[entity]_
* __Production1__: 
`TK_FUNCTION TK_ID TK_TAKES id_list TK_RETURNS idref_expr function_body`
  - __ANK__: `ANK_TRIPLE_CHILD_WDATA` 
  - __ANS__: `ANS_FUNCTION` 
  - __Data__: `TK_ID` identifier name 
  - __Child0__: params `id_list` 
  - __Child1__: return values `idref_expr` 
  - __Child2__: `function_body` 
* __Production2__:
`TK_FUNCTION TK_ID TK_TAKES id_list TK_RETURNS function_body`
  - __ANK__: `ANK_TRIPLE_CHILD_WDATA` 
  - __ANS__: `ANS_FUNCTION` 
  - __Data__: `TK_ID` identifier name 
  - __Child0__: params `id_list` 
  - __Child1__: `NULL`
  - __Child2__: `function_body` 

### `function_body` node _[wrapper]_
* __Production__: `TK_BEGIN statements TK_END` 
* __ANK__: `ANK_SINGLE_CHILD` 
* __ANS__: `ANS_FUNCTION_BODY` 
* __Child0__: `statements` 

### `statements` node _[list]_
* __Production__: `statements statement | <empty>` 
* __ANK__: `ANK_LIST` 
* __ANS__: `ANS_STATEMENTS` 
* __Children__: `statement`s 

### `statement` node _[shell]_
* __Production__: 
`expr_statement | if_statement | while_statement | for_statement | 
empty_statement` 

### `expr_statement` node _[shell]_
* __Production__: `expr TK_SYM_SEMI` 

### `expr` node _[shell]_
* __Production__: `assign_expr`

### `assign_expr` node _[entity or shell]_
* __Production1__: `assign_expr TK_ESYM_EQ logic_expr` _[entity]_ 
  - __ANK__: `ANK_DUAL_CHILD_WDATA` 
  - __ANS__: `ANS_BINEXPR`
  - __Child1__: left hand side `assign_expr`
  - __Child2__: right hand side `logic_expr`
  - __Data__: with `ivalue` field valid, `TK_ESYM_EQ`
* __Production2__: `logic_expr` _[shell]_

### `logic_expr` node _[entity or shell]_
* __Production1__: `logic_expr bin_logicop rel_expr` _[entity]_
  - __ANK__: `ANK_DUAL_CHILD_WDATA`
  - __ANS__: `ANS_BINEXPR`
  - __Child1__: left hand side `logic_expr`
  - __Child2__: right hand side `rel_expr`
  - __Data__: with `ivalue` field valid, value of `bin_logicop`
* __Production2__: `rel_expr` _[shell]_

### `rel_expr` node _[entity or shell]_
* __Production1__: `rel_expr bin_relop add_expr` _[entity]_
  - __ANK__: `ANK_DUAL_CHILD_WDATA`
  - __ANS__: `ANS_BINEXPR`
  - __Child1__: left hand side `rel_expr`
  - __Child2__: right hand side `add_expr`
  - __Data__: with `ivalue` field valid, value of `bin_relop`
* __Production2__: `add_expr` _[shell]_

### `add_expr` node _[entity or shell]_
* __Production1__: `add_expr bin_addop mul_expr` _[entity]_
  - __ANK__: `ANK_DUAL_CHILD_WDATA`
  - __ANS__: `ANS_BINEXPR`
  - __Child1__: left hand side `add_expr`
  - __Child2__: right hand side `mul_expr`
  - __Data__: with `ivalue` field valid, value of `bin_addop`
* __Production2__: `mul_expr` _[shell]_

### `mul_expr` node _[entity or shell]_
* __Production1__: `mul_expr mul_relop unary_expr` _[entity]_
  - __ANK__: `ANK_DUAL_CHILD_WDATA`
  - __ANS__: `ANS_BINEXPR`
  - __Child1__: left hand side `mul_expr`
  - __Child2__: right hand side `unary_expr`
  - __Data__: with `ivalue` field valid, value of `bin_mulop`
* __Production2__: `unary_expr` _[shell]_

### `unary_expr` node _[shell]_
* __Production__: `atom_expr`
> TODO: there are bugs in unary_expr parsing. Update documentation after
fixing that bug


### `atom_expr` node _[shell]_
* __Production1__: `int_expr | float_expr | idref_expr | call_expr`
* __Production2__: `TK_LBRACKET expr TK_RBRACKET`
  - __Inherit from__: `expr`
  
### `int_expr` node _[wrapper]_
* __Production__: `TK_NUM_INT`
* __ANK__: `ANK_LEAF_WDATA`
* __ANS__: `ANS_INTVAL`
* __Data__: with `ivalue` field valid, value of `TK_NUM_INT`

### `float_expr` node _[wrapper]_
* __Production__: `TK_NUM_FLOAT`
* __ANK__: `ANK_LEAF_WDATA`
* __ANS__: `ANS_FLOATVAL`
* __Data__: with `fvalue` field valid, value of `TK_NUM_FLOAT`

### `idref_expr` node _[wrapper]_
* __Production__: `TK_ID`
* __ANK__: `ANK_LEAF_WDATA`
* __ANS__: `ANS_IDREF`
* __Data__: with `svalue` field valid, value of `TK_ID`

### `str_expr` node _[wrapper]_
* __Production__: `TK_STR`
* __ANK__: `ANK_LEAF_WDATA`
* __ANS__: `ANS_STR`
* __Data__: with `svalue` field valid, value of `TK_ID`

### `func_call_expr` node _[wrapper]_
* __Production__:
`idref_expr TK_SYM_LBRACKET semi_sep_list TK_SYM_RBRACKET`
* __ANK__: `ANK_DUAL_CHILD`
* __ANS__: `ANS_FUNC_CALL`
* __Child0__: the called function `idref_expr`
* __Child1__: the argument list `semi_sep_list`

### `if_stmt` node _[entity]_
* __Production1__: 
`TK_IF expr TK_THEN statements TK_ELSE statements TK_END TK_IF`
  - __ANK__: `ANK_TRIPLE_CHILD`
  - __ANS__: `ANS_IF`
  - __Child0__: condition `expr`
  - __Child1__: then clause `statements`
  - __Child2__: else clause `statements`
* __Production2__:
`TK_IF expr TK_THEN statements TK_END TK_IF`
  - __ANK__: `ANK_DUAL_CHILD`
  - __ANS__: `ANS_IF`
  - __Child0__: condition `expr`
  - __Child1__: then clause `statements`

### `while_stmt` node _[entity]_
* __Production__:  `TK_WHILE expr TK_THEN statements TK_END TK_WHILE`
* __ANK__: `ANK_DUAL_CHILD`
* __ANS__: `ANS_WHILE`
* __Child0__: condition `expr`
* __Child1__: while clause `statements`

### `for_stmt` node _[entity]_
* __Production__: 
`TK_FOR idref_expr TK_ESYM_EQ int_expr TK_TO int_expr TK_THEN
 statements TK_END TK_FOR`
* __ANK__: `ANK_DUAL_CHILD`
* __ANS__: `ANS_FOR`
* __Child0__: _anonymous node "for head"_
* __Child1__: for clause `statements`

### _anonymous node "for head"_
* __Production__: 
`TK_FOR idref_expr TK_ESYM_EQ int_expr TK_TO int_expr TK_THEN`
* __ANK__: `ANK_TRIPLE_CHILD`
* __ANS__: `ANS_FOR_HEAD`
* __Child0__: for loop variable `idref_expr`
* __Child1__: start from `int_expr`
* __Child2__: stop at `int_expr`
