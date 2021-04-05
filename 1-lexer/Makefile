# build cli lexer
  all: clean lexer lexer-cli.c
	gcc lexer-cli.c lex.yy.c ../charutil.c ../semval.c -o lex -Wall -Wextra -Wpedantic -std=gnu11 -I../

  lexer: lexer.l
	flex lexer.l

  test:
	gcc -E ltests/*c | ./lex | diff -b ltests/ltest.out -

  clean:
	$(RM) lex lex.yy.c
