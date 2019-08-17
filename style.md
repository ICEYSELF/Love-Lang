# PiLang Coding Styles

### Two spaces indent
Always use two spaces for indents.

### 73 characters per line at most
PiLang is a programming language for small devices like raspberry pi.
These devices may not have large screens. If a user want to contribute
to PiLang, he does not have to get a large screen to write codes if we
limit one line to 73 characters. Not only codes but also documents
shall obey this rule.

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
