all: gast.html pilang-ast.html memmodel.html pli-storage.html

gast.html: gast.md
	markdown gast.md > gast.html
	
pilang-ast.html: pilang-ast.md
	markdown pilang-ast.md > pilang-ast.html

memmodel.html: memmodel.md
	markdown memmodel.md > memmodel.html

pli-storage.html: pli-storage.md
	markdown pli-storage.md > pli-storage.html

clean:
	rm -rf *.html
