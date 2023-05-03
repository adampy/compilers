gcc -std=c99 -g lexer.h parser.h symbols.h compiler.h vmwriter.h lexer.c parser.c symbols.c compiler.c vmwriter.c -o compiler;
./compiler;