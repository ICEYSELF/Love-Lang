# PiLang Memory Model Documentation

## Storage duration and storage spaces
### Stack and auto storage duration
All local objects with simple types: Int, Float, Str and Ref are stored
on stack. Stack storage starts when object gets created and stops when
function exits.

### Heap and dynamic storage duration
All objects with complicated data types: List, Object are stored on 
heap. Heap storage starts when object gets created and stops when it is
no more needed. Garbage collection machanism will automatically cleanup
unused objects.

### Garbage Collection
> This part is incomplete by this time.

### Memory Expansion
> This part is incomplete by this time.

### Multiple Stack
PiLang implementation allows multiple stacks to exist at a time. This is
bogus but maybe useful when implementing coroutines.

## Shallow assign mode
### Assigning data to stack object
* If the assignee object is a reference to a heap object, then assign 
to heap object.

* Otherwise, If the assigned value has simple type, the assignee simply
drops its original data and stores new data.

* Otherwise, If the assigned value is a reference to heap object, the
stack object shall drop its original data and become a reference to that
heap object.

### Assigning data to heap object
When assigning data to heap object, the heap object drops its original
value and takes the new value. If the assignee is a reference, deref it
automatically.

## Deep assign mode
### Assigning data to stack object
* If the assigned value is a reference to heap object, first
create a copy of the heap object, then let the stack object drop its 
original data and become a reference to the newly created heap object.

* Otherwise, the rules are the same as stated before.

### Deep copying heap object
* If the heap object has list value, deep copy its each element.

* Otherwise, if the heap object has object value, deep copy all fields
of the set.

* Otherwise, simply copy "simply typed" data.
