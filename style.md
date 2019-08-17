# Love-Lang Coding Styles

### Two spaces indent
Always use two spaces for indents.

### 80 characters per line at most

### Lowercase, underscore
Always use lower case for identifiers except for macros, and use
underscore to separate meaningful parts.

### Left braces
Place left brace at the end of a line. for example
```
  void handle_stuff() {
    if (something_happens()) {
      do_something();
    }
  }
```

### Prefer type names with `_t` appendix
for example
```
  jjvalue_t
  plvalue_t
  stkobj_t
```
