%{
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include "shell.h"
#include "../GRPCClient.h"
#include "../SimpleResolutionClient.h"
#include "../SimpleKeyMapper.h"
#include "../SimpleTransactionIDGenerator.h"
#include "../SimpleTimestampGenerator.h"
#include "../Coordinator.h"

using namespace std;

// Functions and Variables from Flex that Bison needs.
extern int yylex();
extern FILE *yyin;
extern FILE *yyout;

// Error function declaration.
void yyerror(const char *s);

const char *typeNames[] = { "VAR", "NUM", "VALUE", "GET", "PUT", "ASSIGN", "COMMAND", "START", "END", "DEF", "RUN" };

const string prompt("LegoShell>");
GRPCClient rpc_client;
SimpleResolutionClient rsl_client(&rpc_client);
SimpleKeyMapper client_key_mapper;
SimpleTransactionIDGenerator id_gen;
SimpleTimestampGenerator ts_gen;
Coordinator *coord;

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
              {
                delete coord;
              }
            | READ NUMBER
              {
                coord = new Coordinator(&rsl_client, &client_key_mapper, &id_gen, &ts_gen);
                cout << *(coord->read(uint64_t ($2))) <<  endl;
                coord->commit();
                delete coord;
              }
            | WRITE NUMBER STRING
              {
                coord = new Coordinator(&rsl_client, &client_key_mapper, &id_gen, &ts_gen);
                coord->write((uint64_t) $2, $3);
                coord->commit();
                delete coord;
              }
            | WRITE NUMBER NUMBER
              {
                coord = new Coordinator(&rsl_client, &client_key_mapper, &id_gen, &ts_gen);
                coord->write((uint64_t) $2, new string(std::to_string($3)));
                coord->commit();
                delete coord;
              }
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
                    coord = new Coordinator(&rsl_client, &client_key_mapper, &id_gen, &ts_gen);
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
            cout << *(coord->read(uint64_t ($2))) <<  endl;
          }
        | WRITE NUMBER STRING
          {
            coord->write((uint64_t) $2, $3);
            
          }
        | WRITE NUMBER NUMBER
          {
            coord->write((uint64_t) $2, new string(std::to_string($3)));
          }
        ;
        
end_sequence    : COMMIT NEWLINE
                  {
                    if (coord->commit())
                      cout << "Committed" << endl;
                    else
                      cout << "Aborted" << endl;
                    cout << prompt;
                  }
                | ABORT NEWLINE
                  {
                    coord->abort();
                    cout << prompt;
                  }
                ;

%%

int main(int argc, char** argv)
{ 
  // Parse
  cout << prompt;
  yyparse();
  return 0;
}

void yyerror(const char *s) {
	cout << "Parse error: " << s << endl;
    help();
}
