#include "mtrc-parser.h"
#include "scan.flex.h"
#include "grammar.tab.h"

turing_mashine* turingMashine;

void do_parse(turing_mashine*& tur, const char* filename)
{
	turingMashine = NULL;
	if(fopen_s(&yyin, filename, "rt"))
		yyin = stdin;

	yyparse();

	if(yyin != stdin)
		fclose(yyin);
	tur = turingMashine;
}