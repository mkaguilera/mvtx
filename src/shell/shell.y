%{
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include "shell.h"

using namespace std;

// Functions and Variables from Flex that Bison needs.
extern int yylex();
extern FILE *yyin;
extern FILE *yyout;

// Error function declaration.
void yyerror(const char *s);

const char *typeNames[] = { "VAR", "NUM", "VALUE", "GET", "PUT", "ASSIGN", "COMMAND", "START", "END", "DEF", "RUN" };

const string prompt("LegoShell>");

void help() {
  cout << "READ: read <key>" << endl;
  cout << "WRITE: write <key> <value>" << endl;
  cout << "TRX: begin <\\n> ((READ | WRITE) <\\n>)+ (commit | abort) <\\n>" << endl;
  cout << "EXIT: exit" << endl;
}

%}

%union{
    uint64_t number;
    std::string *string;
}

%token START END READ WRITE COMMIT ABORT EXIT HELP NEWLINE
%token <number> NUMBER 
%token <string> STRING

%%

program : transaction | program transaction;  

transaction : start_sequence commands end_sequence
            | command
            | EXIT
              {
                exit(0);
              }
            | HELP
              {
                help();
              }
            | NEWLINE
              {
                cout << prompt;
              }
            | error
              {
              }
            ;

start_sequence  : START NEWLINE
                  {
                    cout << prompt << ">";
                  }
                ;

commands    : command NEWLINE
              {
                cout << prompt << ">";
              }
            | commands command NEWLINE
              {
                cout << prompt << ">";
              }
            ;

command : READ NUMBER
          {
            cout << "Executing read(" << $2 << ")..." << endl;
            cout << "Result:0" << endl;
          }
        | WRITE NUMBER STRING
          {
            cout << "Executing write(" << $2 << ",\"" << *($3) << "\")..." << endl;
          }
        | WRITE NUMBER NUMBER
          {
            cout << "Executing write(" << $2 << ",\"" << $3 << "\")..." << endl;
          }
        ;
        
end_sequence    : COMMIT NEWLINE
                  {
                    cout << "Executing commit..." << endl;
                    cout << "Result:Committed" << endl;
                    cout << prompt;
                  }
                | ABORT NEWLINE
                  {
                    cout << "Executing abort..." << endl;
                    cout << "Result:Aborted" << endl;
                    cout << prompt;
                  }
                ;

%%

int main(int argc, char** argv)
{
    cout << prompt;
    yyparse();
    return 0;
}

void yyerror(const char *s) {
	cout << "Parse error: " << s << endl;
    help();
}
