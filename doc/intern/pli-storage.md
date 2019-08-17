# PiLang Intepreter storage documentation

## Storage classes
As PiLang Memory Model Documentation specifies, there are two kinds of
storage spaces:

* Stack storage and automatic storage duration
* Heap storage and dynamic storage duration

What's more, in order to store some intermediate values of calculations,
PLI uses another assist storage

* Temporary storage and dying storage duration

## Data structures
### Common sense
All "storage" structures must include a `jjvalue_t` for holding concrete
values and an "object ID" for holding extra type info.

### Temporary storage: `plvalue_t`
A `plvalue_t` denotes result of one calculation. A `plvalue_t` may be
a(an):

* "temporary" object which hold a value directly

In such circumstance, `roc` should be `ROC_TEMP`, and values are stored
directly in `data` field. Only "light weight" or "value typed" objects
can be stored in a `plvalue_t` directly.

* Reference to stack value

In such circumstance, `roc` should be `ROC_ONSTACK`, and `data.pvalue`
holds the pointer to that stack object. `type` simply "forwards" type
info (`soid`) held by the stack object.

* Reference to heap value

In such circumstance, `roc` should be `ROC_ONHEAP`, and `data.pvalue`
holds the pointer to that heap object. `type` simply "forwards" type
info (`oid`) held by the heap object.
