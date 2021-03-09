# build cli lexer
  all: clean lexer lexer-cli.c
	clang lexer-cli.c lex.yy.c ../charutil.c -o lex -std=gnu99 -I../

  lexer: lexer.l
	flex lexer.l

  test:
	gcc -E ltests/*c | ./lex | diff -b ltests/ltest.out - | wc -l

  clean:
	$(RM) lex lex.yy.c
