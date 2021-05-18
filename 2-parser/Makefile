# build cli parser
  all: clean parser lexer
	clang -fsanitize=undefined -I../ -I../3-quads/*.h -I ../target_x86/ -I./ parser.tab.c ast.c symtab.c types.c ../1-lexer/lex.yy.c ../*.c  ../3-quads/quad*c ../target_x86/*.c -o dcc -Wpedantic -Wall -Wextra -std=gnu2x -rdynamic -Wunused
	ln -s dcc parse

  lexer: ../1-lexer/lexer.l
	flex -o ../1-lexer/lex.yy.c ../1-lexer/lexer.l

  parser: parser.y
	bison parser.y -d --verbose

  test:
	gcc -E exprtests.c | ./parse

  clean:
	$(RM) parse parser.tab.c
