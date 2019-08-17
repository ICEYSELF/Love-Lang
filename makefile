CFLAGS = -fPIC -Wall -Wextra -Wno-unused-function -std=c99 -g

all: bins scripts libs stdlib tests

bins: llrepl.bin lli.bin llast.bin

scripts: lli llast llrepl

stdlib: libmstr.so

libs: libllang.so libfront.so libsup.so

llrepl: llrepl.script.in
	cp llrepl.script.in llrepl
	chmod 755 llrepl

lli: lli.script.in
	cp lli.script.in lli
	chmod 755 lli

llast: llast.script.in
	cp llast.script.in llast
	chmod 755 llast

llrepl.bin: llrepl.o libllang.so libfront.so libsup.so
	$(CC) builtins.o llrepl.o \
              dynload.o -L. -lllang -lfront -lsup -ldl -o llrepl.bin

lli.bin: lli.o libllang.so libfront.so libsup.so
	$(CC) lli.o builtins.o dynload.o \
              -L. -lllang -lfront -lsup -ldl -o lli.bin

llast.bin: llast.o libfront.so libsup.so
	$(CC) llast.o -L. -lfront -lsup -ldl -o llast.bin

tests: test_stack test_gc test_calc test_assign test_eval_expr

libmstr.so: mutstr.o libllang.so libsup.so
	$(CC) mutstr.o -L. -lllang -lsup -shared -o libmstr.so

mutstr.o: stdlib/mutstr.c
	$(CC) stdlib/mutstr.c -I include -I ./ -c -o mutstr.o \
              $(CFLAGS)

libfront.so: lexer.o parser.o ast.o tree_dump.o
	$(CC) lexer.o parser.o ast.o tree_dump.o -shared -o libfront.so

libllang.so: value.o stack.o plheap.o eval.o builtins.o
	$(CC) value.o stack.o plheap.o eval.o builtins.o \
              -shared -o libllang.so

libsup.so: clist.o mstring.o dynload.o
	$(CC) clist.o mstring.o dynload.o -shared -o libsup.so

test_eval_expr: test_eval_expr.o libllang.so libfront.so libsup.so
	$(CC) test_eval_expr.o \
              -L. -lllang -lfront -lsup -ldl -o test_eval_expr

test_assign: test_assign.o libllang.so libsup.so
	$(CC) test_assign.o -L. -lllang -lsup -ldl -o test_assign

test_calc: test_calc.o libllang.so libsup.so
	$(CC) test_calc.o -L. -lllang -lsup -ldl -o test_calc

test_gc: test_gc.o plheap.o libsup.so
	$(CC) test_gc.o plheap.o -L. -lsup -ldl -o test_gc

test_stack: stack.o test_stack.o
	$(CC) stack.o test_stack.o -L. -lsup -ldl -o test_stack

test_eval_expr.o: test/test_eval_expr.c y.tab.h
	$(CC) -c -I include -I . test/test_eval_expr.c -o \
        test_eval_expr.o $(CFLAGS)

test_assign.o: test/test_assign.c
	$(CC) -c -I include test/test_assign.c \
              -o test_assign.o $(CFLAGS)

test_calc.o: test/test_calc.c
	$(CC) -c -I include test/test_calc.c -o test_calc.o $(CFLAGS)

test_gc.o: test/test_gc.c
	$(CC) -c -I include test/test_gc.c -o test_gc.o $(CFLAGS)

test_stack.o: test/test_stack.c
	$(CC) -c -I include test/test_stack.c -o test_stack.o $(CFLAGS)

llast.o: src/driver/llast.c y.tab.h
	$(CC) -c -I include -I ./ src/driver/llast.c \
              -o llast.o $(CFLAGS)

lli.o: src/driver/lli.c y.tab.h
	$(CC) -c -I include -I ./ src/driver/lli.c -o lli.o $(CFLAGS)

llrepl.o: src/driver/llrepl.c y.tab.h
	$(CC) -c -I include -I ./ src/driver/llrepl.c -o llrepl.o \
        $(CFLAGS)

dynload.o: src/support/dynload_posix.c y.tab.h
	$(CC) -c -I include -I ./ src/support/dynload_posix.c \
        -o dynload.o $(CFLAGS)

builtins.o: src/eval/builtins.c y.tab.h
	$(CC) -c -I include -I ./ src/eval/builtins.c \
        -o builtins.o $(CFLAGS)

value.o: src/eval/value.c y.tab.h
	$(CC) -c -I include -I ./ src/eval/value.c \
        -o value.o $(CFLAGS)

eval.o: src/eval/eval.c y.tab.h
	$(CC) -c -I include -I ./ src/eval/eval.c \
        -o eval.o $(CFLAGS)

stack.o: src/eval/stack.c
	$(CC) -c -I include src/eval/stack.c -o stack.o $(CFLAGS)

plheap.o: src/eval/heap.c
	$(CC) -c -I include src/eval/heap.c -o plheap.o $(CFLAGS)

tree_dump.o: src/ast/tree_dump.c y.tab.h
	$(CC) -c -I include -I ./ src/ast/tree_dump.c \
        -o tree_dump.o $(CFLAGS)

ast.o : src/ast/ast.c
	$(CC) -c -I include src/ast/ast.c -o ast.o $(CFLAGS)

clist.o : src/support/clist.c
	$(CC) -c -I include src/support/clist.c \
              -o clist.o $(CFLAGS)

mstring.o : src/support/mstring.c
	$(CC) -c -I include src/support/mstring.c \
              -o mstring.o $(CFLAGS)

parser.o : y.tab.c
	$(CC) -c -I include y.tab.c -o parser.o $(CFLAGS)

lexer.o : src/frontend/lexer.c y.tab.h
	$(CC) -c -I include src/frontend/lexer.c -I ./ \
              -o lexer.o $(CFLAGS)

y.tab.c y.tab.h : src/frontend/parser.y
	yacc -d -v src/frontend/parser.y
	if [ -f parser.tab.h ]; then mv parser.tab.h y.tab.h; fi
	if [ -f parser.tab.c ]; then mv parser.tab.c y.tab.c; fi

clean:
	rm -rf y.tab.*
	rm -rf y.output
	rm -rf *.o
	rm -rf lli llast llrepl
	rm -rf *.bin
	rm -rf test_*
	cd doc && make clean && cd ..
	rm -rf doc/docs
	rm -rf *.so
	rm -rf *.a
