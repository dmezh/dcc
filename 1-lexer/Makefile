# build cli lexer
  all: clean lexer lexer-cli.c
	gcc lexer-cli.c -o lex -std=gnu99

  lexer: lexer.l
	flex lexer.l

  clean:
	$(RM) lex lex.yy.c
