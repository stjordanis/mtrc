%{
#include <unordered_map>
#include <stdexcept>
int yylex();
#include "nodes\turing.h"
#include "grammar.tab.h"
static int line_num = 1;
int get_id(const char* str);
%}

LETTER	[A-Za-z_\200-\377]
DIGIT	[0-9]
IDNAME	{LETTER}({LETTER}|{DIGIT})*

%x comment

%%
<INITIAL,comment>\n		line_num++;
"/*"					BEGIN(comment);
<comment>[^*\n]*		/* eat anything not a '*' */
<comment>"*"+[^*/\n]*	/* eat up '*'s not followed by '/'s */
<comment>"*"+"/"		BEGIN(INITIAL);
"//".*			/* ignore C++-style comments */
[ \t\r\v\n\f]	/* ignore whitespace */
"\xEF\xBB\xBF"	/* ignore BOM */
alpha			{ return(Alpha); }
begin			{ return(Begin); }
dir				{ return(Dir); }
else			{ return(Else); }
empty			{ return(Empty); }
end				{ return(End); }
first			{ return(First); }
goto			{ return(Goto); }
if				{ return(If); }
left			{ return(Left); }
move			{ return(Move); }
multiline		{ return(Multiline); }
not				{ return(Not); }
right			{ return(Right); }
second			{ return(Second); }
state			{ return(State);  }
symbol			{ return(Symbol); }
template		{ return(Template); }
turing			{ return(Turing); }
value			{ return(Value); }
{DIGIT}+		{ yylval.i = atoi(yytext); return(STATE_NUMBER); }
{IDNAME}		{ yylval.i = get_id(yytext); return(IDENTIFIER); }
'(\\.|[^\\'])'	{ yylval.sym = (unsigned char)(((yytext[1] == '\\')? yytext[2] : yytext[1])); return(SYMBOL); }
"..."			{ return(ELLIPSIS); }
"=="			{ return(EQU); }
"!="			{ return(NEQ); }
">="			{ return(GEQ); }
"<="			{ return(LEQ); }
";"				{ /* count(); */ return(';'); }
("{"|"<%")		{ /* count(); */ return('{'); }
("}"|"%>")		{ /* count(); */ return('}'); }
","				{ /* count(); */ return(','); }
":"				{ /* count(); */ return(':'); }
"="				{ /* count(); */ return('='); }
"("				{ /* count(); */ return('('); }
")"				{ /* count(); */ return(')'); }
"<"				{ /* count(); */ return('<'); }
">"				{ /* count(); */ return('>'); }
.				/* ignore bad characters */;

%%
void yyerror(const char *str)
{
	fprintf(stderr, "line %d: %s\n", line_num, str);
}


int yywrap()
{
	return(1);
}

int get_id(const char* str)
{
	static std::unordered_map<std::string, int> id_table;
	static int id_count = 0;
	int id;
	try
	{
		id = id_table.at(str);
	}
	catch(std::out_of_range)
	{
		id = id_count;
		id_table[str] = id_count;
		id_count++;
	}
	return id;
}