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
exit            { return EXIT; }
help            { return HELP; }
\n              { return NEWLINE; }
[0-9]+          {
                    yylval.key = std::strtoul (yytext, NULL, 10);
                    return KEY;
                }
[a-zA-Z0-9]+    {
                    yylval.value = new std::string(yytext);
                    return VALUE;
                }
.               {}

%%
