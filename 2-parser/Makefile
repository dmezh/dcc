# build cli parser
  all: clean parser
	clang parser.tab.c ast.c ../1-lexer/lex.yy.c ../charutil.c -I../ -o parse -Wpedantic -std=gnu11

  parser: parser.y
	bison -Wcex parser.y -d --verbose

  test:
	gcc -E exprtests.c | ./parse

  clean:
	$(RM) parse parser.tab.c
