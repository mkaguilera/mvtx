all: shell.exe

shell.tab.c shell.tab.h : shell.y shell.h
	bison -o shell.tab.c -vd shell.y

shell.lex.c : shell.l shell.tab.h
	flex -o shell.lex.c -l shell.l

shell.exe: shell.lex.c shell.tab.c shell.tab.h
	g++ -std=c++11 -o legoShell shell.lex.c shell.tab.c -lfl

clean:
	rm -f legoShell shell.tab.* shell.lex.*
