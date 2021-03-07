# build cli lexer
  all: clean lexer lexer-cli.c
	clang lexer-cli.c -o lex -std=gnu99 -I../

  lexer: lexer.l
	flex lexer.l

  clean:
	$(RM) lex lex.yy.c
