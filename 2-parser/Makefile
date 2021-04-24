# build cli parser
  all: clean parser lexer
	clang -fsanitize=undefined parser.tab.c ast.c symtab.c types.c ../1-lexer/lex.yy.c ../*.c -I../ -o parse -Wpedantic -Wall -Wextra -std=gnu11 -rdynamic -Wunused

  lexer: ../1-lexer/lexer.l
	flex -o ../1-lexer/lex.yy.c ../1-lexer/lexer.l

  parser: parser.y
	bison parser.y -d --verbose

  test:
	gcc -E exprtests.c | ./parse

  clean:
	$(RM) parse parser.tab.c
