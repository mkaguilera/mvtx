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

const std::string prompt("LegoShell>");


/*
Node *makeCharNode(char c) {
	Node *node = (Node *) malloc(sizeof(Node));

	node->type = TCHAR;
	node->c = c;
	node->length = 0;

	return node;
}

Node *makeRangeNode(char s, char e) {
	Node *node = (Node *) malloc(sizeof(Node));
	
	node->type = TRANGE;
	node->length = 2;
	node->children = (Node **) malloc(node->length*sizeof(Node*));
	node->children[0] = makeCharNode(s);
	node->children[1] = makeCharNode(e);
	
	return node;
}

Node *makeZeroChildNode(Type type) {
	Node *node = (Node *) malloc(sizeof(Node));
	
	node->type = type;
	node->length = 0;
	
	return node;
}

Node *makeOneChildNode(Type type, Node *a) {
	Node *node = (Node *) malloc(sizeof(Node));
	
	node->type = type;
	node->length = 1;
	node->children = (Node **) malloc(node->length*sizeof(Node*));
	node->children[0] = a;
	
	return node;
}

Node *makeTwoChildrenNode(Type type, Node *a, Node *b) {
	Node *node = (Node *) malloc(sizeof(Node));
	
	node->type = type;
	node->length = 2;
	node->children = (Node **) malloc(node->length*sizeof(Node*));
	node->children[0] = a;
	node->children[1] = b;
	
	return node;
}

Node *makeThreeChildrenNode(Type type, Node *a, Node *b, Node *c) {
	Node *node = (Node *) malloc(sizeof(Node));
	
	node->type = type;
	node->length = 3;
	node->children = (Node **) malloc(node->length*sizeof(Node*));
	node->children[0] = a;
	node->children[1] = b;
	node->children[2] = c;
	
	return node;
}

Node *extendSequence(Node *prevNode, Node *newChild) {
	Node *node = (Node *) malloc(sizeof(Node));

	node->type = prevNode->type;
	node->length = prevNode->length + 1;
	node->children = (Node **) malloc(node->length*sizeof(Node*));
	for (int i = 0; i < prevNode->length; i++)
		node->children[i] = prevNode->children[i];
	node->children[prevNode->length] = newChild;
	free(prevNode);

	return node;
}

Node *setType(Node *prevNode, Type newType) {
	Node *node = prevNode;
	
	node->type = newType;

	return node;
}

void printNode(Node *node) {
	fprintf(yyout, "%s(", typeNames[node->type]);
	if ((node->type == TVAR) || (node->type == TVALUE)) {
		fprintf(yyout, "%s)", node->s);
	} else if (node->type == TNUM) {
	    fprintf(yyout, "%d)", node->n);
	} else {
		for (int i = 0; i < node->length-1; i++) {
			printNode(node->children[i]);
			fprintf(yyout, ",");
		}
		if (node->length != 0)
			printNode(node->children[node->length-1]);
		fprintf(yyout, ")");
	}
}*/
%}

%union{
    uint64_t key;
    std::string *value;
}

%token START END READ WRITE COMMIT EXIT HELP NEWLINE
%token <key> KEY
%token <value> VALUE

%%

// definition : start_sequence command_sequence end_sequence;

program : transaction | program transaction;  

transaction : start_sequence commands end_sequence
            | command
            | EXIT
              {
                exit(0);
              }
            | HELP
              {
                cout << "READ: read <key>" << endl;
                cout << "WRITE: write <key> <value>" << endl;
                cout << "TRX: begin <\\n> ((READ | WRITE) <\\n>)+ commit <\\n>" << endl;
                cout << "EXIT: exit" << endl;
              }
            | NEWLINE
              {
                cout << prompt;
              }
            ;

start_sequence  : START NEWLINE

commands    : command NEWLINE
            | commands command NEWLINE
            ;

command : READ KEY
          {
            cout << "Executing read(" << $2 << ")..." << endl;
            cout << "Result:0" << endl;
          }
        | WRITE KEY VALUE
          {
            cout << "Executing write(" << $2 << "," << *($3) << ")..." << endl;
          }
        ;
        
end_sequence    : COMMIT NEWLINE
                  {
                    cout << "Executing commit..." << endl;
                    cout << "Result:Committed" << endl;
                    cout << prompt;
                  }
                ;
                   
              
/*
          {
            printNode($1);
            fprintf(yyout, "\n");
          }
        | run
          {
            printNode($2);
            fprintf(yyout, "\n");
          }

program	:	rule
			{
				printNode($1);
                fprintf(yyout, "\n");
			}
		|	program rule
			{
				printNode($2);
				fprintf(yyout, "\n");
			}
		;

rule	:	outputs COLON inputs EOL TAB action EOL
			{
				$$ = makeThreeChildrenNode(TRULE, $1, $3, $6);
			}
		|	outputs spaces COLON inputs EOL TAB action EOL
			{
				$$ = makeThreeChildrenNode(TRULE, $1, $4, $7);
			}
		|	outputs spaces COLON spaces inputs EOL TAB action EOL
			{
				$$ = makeThreeChildrenNode(TRULE, $1, $5, $8);
			}
		|	outputs COLON spaces inputs EOL TAB action EOL
			{
				$$ = makeThreeChildrenNode(TRULE, $1, $4, $7);
			}
		|	outputs COLON inputs spaces EOL TAB action EOL
			{
				$$ = makeThreeChildrenNode(TRULE, $1, $3, $7);
			}
		|	outputs spaces COLON inputs spaces EOL TAB action EOL
			{
				$$ = makeThreeChildrenNode(TRULE, $1, $4, $8);
			}
		|	outputs spaces COLON spaces inputs spaces EOL TAB action EOL
			{
				$$ = makeThreeChildrenNode(TRULE, $1, $5, $9);
			}
		|	outputs COLON spaces inputs spaces EOL TAB action EOL
			{
				$$ = makeThreeChildrenNode(TRULE, $1, $4, $8);
			}
		|	FOREACH spaces var spaces IN spaces LEFT_BRACKET_2 strings RIGHT_BRACKET_2 spaces DO EOL rule
			{
				$$ = makeThreeChildrenNode(TSRULE, $3, $8, $13);
			}
		|	FOREACH spaces var spaces IN spaces LEFT_BRACKET_2 strings RIGHT_BRACKET_2 spaces DO spaces EOL rule
			{
				$$ = makeThreeChildrenNode(TSRULE, $3, $8, $14);
			}
		|	FOREACH spaces var spaces IN spaces LEFT_BRACKET_2 spaces strings RIGHT_BRACKET_2 spaces DO EOL rule
			{
				$$ = makeThreeChildrenNode(TSRULE, $3, $9, $14);
			}
		|	FOREACH spaces var spaces IN spaces LEFT_BRACKET_2 spaces strings RIGHT_BRACKET_2 spaces DO spaces EOL rule
			{
				$$ = makeThreeChildrenNode(TSRULE, $3, $9, $15);
			}
		|	FOREACH spaces var spaces IN spaces LEFT_BRACKET_2 strings spaces RIGHT_BRACKET_2 spaces DO EOL rule
			{
				$$ = makeThreeChildrenNode(TSRULE, $3, $8, $14);
			}
		|	FOREACH spaces var spaces IN spaces LEFT_BRACKET_2 strings spaces RIGHT_BRACKET_2 spaces DO spaces EOL rule
			{
				$$ = makeThreeChildrenNode(TSRULE, $3, $8, $15);
			}
		|	FOREACH spaces var spaces IN spaces LEFT_BRACKET_2 spaces strings spaces RIGHT_BRACKET_2 spaces DO EOL rule
			{
				$$ = makeThreeChildrenNode(TSRULE, $3, $9, $15);
			}
		|	FOREACH spaces var spaces IN spaces LEFT_BRACKET_2 spaces strings spaces RIGHT_BRACKET_2 spaces DO spaces
		    EOL rule
			{
				$$ = makeThreeChildrenNode(TSRULE, $3, $9, $16);
			}
		|	FOREACH spaces var spaces IN spaces LEFT_BRACKET_1 numbers RIGHT_BRACKET_1 spaces DO EOL rule
			{
				$$ = makeThreeChildrenNode(TNRULE, $3, $8, $13);
			}
		|	FOREACH spaces var spaces IN spaces LEFT_BRACKET_1 numbers RIGHT_BRACKET_1 spaces DO spaces EOL rule
			{
				$$ = makeThreeChildrenNode(TNRULE, $3, $8, $14);
			}
		|	FOREACH spaces var spaces IN spaces LEFT_BRACKET_1 spaces numbers RIGHT_BRACKET_1 spaces DO EOL rule
			{
				$$ = makeThreeChildrenNode(TNRULE, $3, $9, $14);
			}
		|	FOREACH spaces var spaces IN spaces LEFT_BRACKET_1 spaces numbers RIGHT_BRACKET_1 spaces DO spaces EOL rule
			{
				$$ = makeThreeChildrenNode(TNRULE, $3, $9, $15);
			}
		|	FOREACH spaces var spaces IN spaces LEFT_BRACKET_1 numbers spaces RIGHT_BRACKET_1 spaces DO EOL rule
			{
				$$ = makeThreeChildrenNode(TNRULE, $3, $8, $14);
			}
		|	FOREACH spaces var spaces IN spaces LEFT_BRACKET_1 numbers spaces RIGHT_BRACKET_1 spaces DO spaces EOL rule
			{
				$$ = makeThreeChildrenNode(TNRULE, $3, $8, $15);
			}
		|	FOREACH spaces var spaces IN spaces LEFT_BRACKET_1 spaces numbers spaces RIGHT_BRACKET_1 spaces DO EOL rule
			{
				$$ = makeThreeChildrenNode(TNRULE, $3, $9, $15);
			}
		|	FOREACH spaces var spaces IN spaces LEFT_BRACKET_1 spaces numbers spaces RIGHT_BRACKET_1 spaces DO spaces
		    EOL rule
			{
				$$ = makeThreeChildrenNode(TNRULE, $3, $9, $16);
			}
		;
		
strings :   string
            {
                $$ = makeOneChildNode(TSEQUENCE, $1);
            }
        |   strings COMMA string
            {
                $$ = extendSequence($1, $3);
            }
        |   strings COMMA spaces string
            {
                $$ = extendSequence($1, $4);
            }
        ;

string  :   CLETTER
            {
                Node *node = makeCharNode($1);
                $$ = makeOneChildNode(TSEQUENCE, node);

            }
        |   SLETTER
            {
                Node *node = makeCharNode($1);
                $$ = makeOneChildNode(TSEQUENCE, node);
            }
        |   DIGIT
            {
                Node *node = makeCharNode($1);
                $$ = makeOneChildNode(TSEQUENCE, node);
            }
        |   string CLETTER
            {
                $$ = extendSequence($1,makeCharNode($2));
            }
        |   string SLETTER
            {
                $$ = extendSequence($1,makeCharNode($2));
            }
        |   string DIGIT
            {
                $$ = extendSequence($1,makeCharNode($2));
            }
        ;

numbers :   number COMMA number
            {
                $$ = makeTwoChildrenNode(TSEQUENCE, $1, $3);
            }
        |   number spaces COMMA number
            {
                $$ = makeTwoChildrenNode(TSEQUENCE, $1, $4);
            }
        |   number COMMA spaces number
            {
                $$ = makeTwoChildrenNode(TSEQUENCE, $1, $4);
            }
        |   number spaces COMMA spaces number
            {
                $$ = makeTwoChildrenNode(TSEQUENCE, $1, $5);
            }
        ;

number  :   DIGIT
            {
                $$ = makeOneChildNode(TSEQUENCE, makeCharNode($1));
            }
        |   number DIGIT
            {
                $$ = extendSequence($1, makeCharNode($2));
            }
       

action	:
			{
				$$ = makeZeroChildNode(TACTION);
			}
		|	action SLASH
			{
				Node *node = makeCharNode('/');
				$$ = extendSequence($1, node);
			}
		|	action LEFT_PARENTHESIS
			{
				Node *node = makeCharNode('(');
				$$ = extendSequence($1, node);
			}
		|	action RIGHT_PARENTHESIS
			{
				Node *node = makeCharNode(')');
				$$ = extendSequence($1, node);
			}
		|	action LEFT_BRACKET_1
			{
				Node *node = makeCharNode('[');
				$$ = extendSequence($1, node);
			}
		|	action RIGHT_BRACKET_1
			{
				Node *node = makeCharNode(']');
				$$ = extendSequence($1, node);
			}
		|	action LEFT_BRACKET_2
			{
				Node *node = makeCharNode('{');
				$$ = extendSequence($1, node);
			}
		|	action RIGHT_BRACKET_2
			{
				Node *node = makeCharNode('}');
				$$ = extendSequence($1, node);
			}
		|	action PLUS
			{
				Node *node = makeCharNode('+');
				$$ = extendSequence($1, node);
			}
		|	action MINUS
			{
				Node *node = makeCharNode('-');
				$$ = extendSequence($1, node);
			}
		|	action ASTERISK
			{
				Node *node = makeCharNode('*');
				$$ = extendSequence($1, node);
			}
		|	action QUESTIONMARK
			{
				Node *node = makeCharNode('?');
				$$ = extendSequence($1, node);
			}
		|	action BAR
			{
				Node *node = makeCharNode('|');
				$$ = extendSequence($1, node);
			}
		|	action CIRCUMFLEX_ACCENT
			{
				Node *node = makeCharNode('^');
				$$ = extendSequence($1, node);
			}
		|	action COLON
			{
				Node *node = makeCharNode(':');
				$$ = extendSequence($1, node);
			}
		|	action DOLLAR
			{
				Node *node = makeCharNode('$');
				$$ = extendSequence($1, node);
			}
		|	action COMMA
			{
				Node *node = makeCharNode(',');
				$$ = extendSequence($1, node);
			}
		|	action TAB
			{
				Node *node = makeCharNode('\t');
				$$ = extendSequence($1, node);
			}
		|	action SPACE
			{
				Node *node = makeCharNode(' ');
				$$ = extendSequence($1, node);
			}
		|   action FOREACH
		    {
		        Node *node = makeCharNode('F');
		        Node *res = $1;
				
				res = extendSequence($1, node);
		        node = makeCharNode('O');
				res = extendSequence(res, node);
		        node = makeCharNode('R');
				res = extendSequence(res, node);
		        node = makeCharNode('E');
				res = extendSequence(res, node);
		        node = makeCharNode('A');
				res = extendSequence(res, node);
		        node = makeCharNode('C');
				res = extendSequence(res, node);
		        node = makeCharNode('H');
				res = extendSequence(res, node);
				$$ = res;
		    }
		|   action IN
		    {
		        Node *node = makeCharNode('I');
		        Node *res = $1;
				
				res = extendSequence($1, node);
		        node = makeCharNode('N');
				res = extendSequence(res, node);
				$$ = res;
		    }
		|   action DO
		    {
		        Node *node = makeCharNode('D');
		        Node *res = $1;
				
				res = extendSequence($1, node);
		        node = makeCharNode('O');
				res = extendSequence(res, node);
				$$ = res;
		    }
		|   action XML_EXTENSION
		    {
		        Node *node = makeCharNode('.');
		        Node *res = $1;
				
				res = extendSequence($1, node);
		        node = makeCharNode('x');
				res = extendSequence(res, node);
		        node = makeCharNode('m');
				res = extendSequence(res, node);
		        node = makeCharNode('l');
				res = extendSequence(res, node);
				$$ = res;
		    }
		|	action char
			{
				$$ = extendSequence($1, $2);
			}
		|	action ANY
			{
				Node *node = makeCharNode($2);
				$$ = extendSequence($1, node);
			}
		;

outputs :   entry
            {
                $$ = makeOneChildNode(TOUTPUTS, $1);
            }
        |   outputs spaces entry
            {
                $$ = extendSequence($1,$3);
            }
        ;

inputs  :   entry
            {
                $$ = makeOneChildNode(TINPUTS, $1);
            }
        |   event
            {
                $$ = makeOneChildNode(TINPUTS, $1);
            }
        |   inputs spaces entry
            {
                $$ = extendSequence($1,$3);
            }
        |   inputs spaces event
            {
                $$ = extendSequence($1,$3);
            }
        ;

entry	:	dirpath name XML_EXTENSION xmlpath
			{
				$$ = makeThreeChildrenNode(TENTRY, $1, $2, $4);
			}
		;

event   :   dirpath name
            {
                $$ = makeTwoChildrenNode(TEVENT, $1, $2);
            }
        ;

dirpath :
            {
                $$ = makeZeroChildNode(TDIRPATH);
            }
        |   dirpath name SLASH
            {
                $$ = extendSequence($1, $2);
            }
        ;

xmlpath	:
			{
				$$ = makeZeroChildNode(TXMLPATH);
			}
		|	xmlpath LEFT_BRACKET_2 name RIGHT_BRACKET_2
			{
			    $$ = extendSequence($1, $3);
			}
		;

name    :   sequence
            {
                $$ = makeOneChildNode(TNAME, $1);
            }
		|	varclause
			{
				$$ = makeOneChildNode(TNAME, $1);
			}
	    |   name varclause sequence
	        {
	            Node *node = extendSequence($1, $2);
	            $$ = extendSequence(node, $3);
	        }
	    |   name varclause
	        {
	            $$ = extendSequence($1, $2);
	        }
	    ;

varclause:	DOLLAR LEFT_PARENTHESIS var RIGHT_PARENTHESIS
			{
				$$ = $3;
			}


sequence	:	atom
				{
					$$ = makeOneChildNode(TSEQUENCE, $1);
				}
			|   atom ASTERISK
				{
					Node *node = makeOneChildNode(TASTERISK, $1);
					$$ = makeOneChildNode(TSEQUENCE, node);
				}
			|   atom PLUS
				{
					Node *node = makeOneChildNode(TPLUS, $1);
					$$ = makeOneChildNode(TSEQUENCE, node);
				}
			|   atom QUESTIONMARK
				{
					Node *node = makeOneChildNode(TQUESTIONMARK, $1);
					$$ = makeOneChildNode(TSEQUENCE, node);
				}
			|	sequence atom
				{
					$$ = extendSequence($1, $2);
				}
			|   sequence atom ASTERISK
				{
					Node *node = makeOneChildNode(TASTERISK, $2);
					$$ = extendSequence($1, node);
				}
			|   sequence atom PLUS
				{
					Node *node = makeOneChildNode(TPLUS, $2);
					$$ = extendSequence($1, node);
				}
			|   sequence atom QUESTIONMARK
				{
					Node *node = makeOneChildNode(TQUESTIONMARK, $2);
					$$ = extendSequence($1, node);
				}
			;

atom:	char
		{
			$$ = $1;
		}
	|	LEFT_BRACKET_1 brseq RIGHT_BRACKET_1
		{
			$$ = setType($2, TCHOICE);
		}
	|	CIRCUMFLEX_ACCENT LEFT_BRACKET_1 brseq RIGHT_BRACKET_1
		{
			$$ = setType($3, TNCHOICE);
		}
    |   LEFT_PARENTHESIS sequence BAR sequence RIGHT_PARENTHESIS
        {
			$$ = makeTwoChildrenNode(TUNION, $2, $4);
        }
    |   LEFT_PARENTHESIS sequence RIGHT_PARENTHESIS
        {
            $$ = makeOneChildNode(TATOM, $2);
        }
	;

char:	CLETTER
		{
            $$ = makeCharNode($1);
        }
    |   SLETTER
        {
            $$ = makeCharNode($1);
        }
    |   DIGIT
        {
            $$ = makeCharNode($1);
        }
    |   OTHER_CHAR
        {
            $$ = makeCharNode($1);
        }
    ;

brseq	:	char
			{
				$$ = makeOneChildNode(TBRSEQ, $1);
			}
		|	brseq char
			{
				$$ = extendSequence($1, $2);
			}
		|	range
			{
				$$ = makeOneChildNode(TBRSEQ, $1);
			}
		|	brseq range
			{
				$$ = extendSequence($1, $2);
			}
		;

range	:	CLETTER MINUS CLETTER
			{
				$$ = makeRangeNode($1, $3);
			}
		|	SLETTER MINUS SLETTER
			{
				$$ = makeRangeNode($1, $3);
			}
		|	DIGIT MINUS DIGIT
			{
				$$ = makeRangeNode($1, $3);
			}
		;

var	:	char
		{
			$$ = makeOneChildNode(TVAR, $1);
		}
	|	var char
		{
			$$ = extendSequence($1, $2);
		}

spaces : SPACE
       | TAB
       | spaces SPACE
       | spaces TAB
       ;
*/

%%


int main(int argc, char** argv)
{
    std::cout << prompt;
    yyparse();
    return 0;
}

void yyerror(const char *s) {
	cout << "Parse error!" << endl << "Message: " << s << endl;
	exit(-1);
}
