%{
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include "shell.h"
#include "shell.tab.h"

using namespace std;
%}

%%

begin           { return START; }
end             { return END; }
read            { return READ; }
write           { return WRITE; }
commit          { return COMMIT; }
abort           { return ABORT; }
exit            { return EXIT; }
help            { return HELP; }
\n              { return NEWLINE; }
[0-9]+          {
                    yylval.number = std::strtoul (yytext, NULL, 10);
                    return NUMBER;
                }
[a-zA-Z0-9]+    {
                    yylval.string = new std::string(yytext);
                    return STRING;
                }
.               {}

%%
