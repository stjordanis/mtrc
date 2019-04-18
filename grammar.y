%{
#include <unordered_map>
#include <stdexcept>
#include "nodes\all_nodes.h"
extern int yylex();
extern void yyerror(const char *str);
extern turing_mashine* turingMashine;
static int indexCurrentState;
std::unordered_map<int, template_unit*> templatesTable;
static int actualTemplateId;
%}



%union
{
	int				i;
	unsigned char	sym;
	code_block	   *node;
	code_list	   *list;
};

/* ---------------- special symbols --------------------- */

%token	ELLIPSIS	/* three dots ... */

/* ---------------- keywords ---------------------------- */

%token	Alpha	Begin		Dir			Else	Empty
%token	End		First		Goto		If		Left
%token	Move	Multiline	Not			Right	Second
%token	State	Symbol		Template	Turing	Value

/* ---------------- combined operators ------------------ */ 

%token	EQU	NEQ	LEQ	GEQ

/* ---------------- identifier and sumbol --------------- */

%token	<sym>	SYMBOL
%token	<i>		STATE_NUMBER IDENTIFIER

/*----------------- operator precedence ----------------- */

%left	EQU
%left	NEQ
%left	LEQ	GEQ

/*----------------- token types ------------------------- */

%type <i> const_dir op_compare
%type <node> action optsetting optmoving optgoing setting 
%type <node> cell_value double_line_value value_expression moving
%type <node> dir_value double_dir single_dir going going_expression 
%type <list> values_list symbol_list symbol_value value_set
%type <node> value_double
%type <node> rule else_block optelse ifelse_block rule_action compare_expr
%type <list> unit_body optrules rulelist 
%type <sym> value_argument
%type <i> state_argument optbegin state_id
%type <node> argument inherited optinherited unit unit_hdr
%type <list> argumentlist
%type <i> parameter_type
%type <node> parameter
%type <list> parameters
%type <i> optmultiline optname 
%type <node> alpha_declarator
%type <list> unitlist

/*----------------- start symbol ------------------------ */

%start turing

%%

/* ----------------------------  turing ---------------------------- */


turing	: /* пусто */
		| turheader turbody 
		| error { if(turingMashine != NULL) delete turingMashine; turingMashine = NULL; }
		;

turheader
		: optmultiline Turing optname
		{
			turingMashine = new turing_mashine;
			turingMashine->setMtrType($1);
		}
		;

turbody	: '{' declars '}' ;

optmultiline
		: /* пусто */	{ $$ = 0; }
		| Multiline		{ $$ = 1; }
		;

optname	: /* пусто */	{ $$ = -1; }
		| IDENTIFIER	{ $$ = $1; }
		;

declars	: alpha_declarator unitlist
		{
			turingMashine->setUnits(dynamic_cast<units_list*>($2));
		}
		;

alpha_declarator
		: Alpha '=' '{' symbol_list '}' ';'
		{
			turingMashine->setAlphaList(dynamic_cast<symbol_list*>($4));
			$$ = $4;
		}
		;

unitlist: unit
		{
			units_list* unitsList = new units_list;
			unitsList->addNodeLast($1);
			$$ = unitsList;
		}
		| unitlist unit
		{
			code_list* unitsList = $1;
			unitsList->addNodeLast($2);
			$$ = unitsList;
		}
		;

/* ----------------------------- unit ----------------------------- */

unit	: unit_hdr	unit_body
		{
			translation_unit* translationUnit = dynamic_cast<translation_unit*>($1);
			translationUnit->setRules(dynamic_cast<rules_list*>($2));
			$$ = translationUnit;
		}
		;

unit_hdr: optbegin State		state_id	optinherited
		{
			state_unit* stateUnit = new state_unit;
			stateUnit->setBegin($1);
			stateUnit->setIndex($3);
			stateUnit->setBaseTemplate(dynamic_cast<template_call*>($4));
			$$ = stateUnit;
		}
		| Template	IDENTIFIER	'(' parameters ')' optinherited
		{
			template_unit* templateUnit = new template_unit;
			/* сохранить идентификатор шаблона и указатель в таблицу шаблонов */
			templatesTable[$2] = templateUnit;
			templateUnit->setParameters(dynamic_cast<parameter_list*>($4));
			templateUnit->setBaseTemplate(dynamic_cast<template_call*>($6));
			$$ = templateUnit;
		}
		;

state_id: STATE_NUMBER { indexCurrentState = $1; $$ = $1; };

optbegin: /* пусто */ { $$ = 0; }
		| Begin { $$ = 1; }
		;

optinherited
		: /* пусто */	{ $$ = NULL; }
		| inherited		{ $$ = $1; }
		;

parameters
		: parameter
		{
			parameter_list* parameterList = new parameter_list;
			parameterList->addNodeLast($1);
			$$ = parameterList;
		}
		| parameters ',' parameter
		{
			code_list* parameterList = $1;
			parameterList->addNodeLast($3);
			$$ = parameterList;
		}
		;

parameter
		: parameter_type		IDENTIFIER
		{
			parameter* param = new parameter;
			param->setIdentifier($2);
			param->setType($1);
			$$ = param;
		}
		;

parameter_type
		: Symbol	{ $$ = 0; }
		| Dir		{ $$ = 1; }
		| State		{ $$ = 2; }
		;

/* --------------   inherited   --------------------- */

inherited
		:	':' IDENTIFIER '(' argumentlist ')'
		{
			template_call* templateCall = new template_call;
				template_unit* templateUnit;
				try
				{
					templateUnit = templatesTable.at($2);
 					actualTemplateId = $2;
					templateCall->setTemplate(templateUnit);
				}
				catch (std::out_of_range)
				{
					/* ошибка: идентификатор шаблона не определен. */
				}
			/* проверить соответствие типов фактичеких и формальных переметров */
			templateCall->setArguments(dynamic_cast<argument_list*>$4);
			$$ = templateCall;
		}
		;

argumentlist
		: argument
		{
			argument_list* argumentList = new argument_list;
			argumentList->addNodeLast($1);
			$$ = argumentList;
		}
		| argumentlist ',' argument
		{
			code_list* argumentList = $1;
			argumentList->addNodeLast($3);
			$$ = argumentList;
		}
		;

argument: state_argument
		{
			going_argument_value* goingArgumentValue = new going_argument_value;
			goingArgumentValue->setStateIndex($1);
			$$ = goingArgumentValue;
		}
		| value_argument
		{
			symbol_argument_value* symbolArgumentValue = new symbol_argument_value;
			symbolArgumentValue->setValue($1);
			$$ = symbolArgumentValue;
		}
		| const_dir
		{
			direction_argument_value* directionArgumentValue = new direction_argument_value;
			directionArgumentValue->setDirCode($1);
			$$ = directionArgumentValue;
		}
		;

state_argument
		: STATE_NUMBER { $$ = $1; }
		| Begin { $$ = 0; }
		| End	{ $$ = -1; }
		;

value_argument
		: SYMBOL	{ $$ = $1; }
		| Empty		{ $$ = 0; }
		| First		{ $$ = 0x16; }
		| Second	{ $$ = 0x17; }
		| ELLIPSIS	{ $$ = 0x15; }
		;


/* ----------------------------  refresh ---------------------------- */

unit_body: '{' optrules '}' { $$ = $2; } ;


optrules: /* пусто */	{ $$ = NULL; }
		| rulelist		{ $$ = $1; }
		;

rulelist: rule
		{
			rules_list* rulesList = new rules_list;
			rulesList->addNodeLast($1);
			$$ = rulesList;
		}
		| rulelist rule
		{
			code_list* rulesList = $1;
			rulesList->addNodeLast($2);
			$$ = rulesList;
		}
		;


/* ----------------------------  rule ------------------------------- */

rule	: value_set ':' rule_action
		{
			turing_rule* turingRule = new turing_rule;
			turingRule->setValues(dynamic_cast<conditions_list*>($1));
			turingRule->setAction(dynamic_cast<actions_block*>($3));
			$$ = turingRule;
		}
		;

rule_action : action | ifelse_block  { $$ = $1; } ;

ifelse_block
		: If compare_expr ':' action optelse
		{
			ifelse_block* ifelseBlock = new ifelse_block;
			ifelseBlock->setCondition(dynamic_cast<compare_expression*>($2));
			ifelseBlock->setIfBlock(dynamic_cast<rule_action*>($4));
			ifelseBlock->setElseBlock(dynamic_cast<rule_action*>($5));
			$$ = ifelseBlock;
		}
		;

optelse	: /* пусто */	{ $$ = NULL; }
		| else_block	{ $$ = $1; }
		;

else_block
		: Else ':' action { $$ = $3; } ;

/* ----------------------- compare expression ----------------------- */

compare_expr
			: First op_compare Second
			{
				compare_expression* compareExpression = new compare_expression;
				compareExpression->setCompOperator($2);
				$$ = compareExpression;
			}
			;

op_compare	: EQU	{ $$ = 5; } 
			| NEQ	{ $$ = 4; } 
			| LEQ	{ $$ = 7; } 
			| GEQ	{ $$ = 2; } 
			| '<'	{ $$ = 3; } 
			| '>'	{ $$ = 6; } 
			;

/* ---------------------------  action  ---------------------------- */

action	: optsetting optmoving optgoing
		{
			rule_action* ruleAction = new rule_action;
			ruleAction->setSetting($1);
			ruleAction->setMoving($2);
			ruleAction->setGoing($3);
			$$ = ruleAction;
		}
		;

optsetting
		: /* пусто */
		{
			double_rule* doubleRule = new double_rule;
			doubleRule->setAtLine(new symbol_const, 0);
			doubleRule->setAtLine(new symbol_const, 1);
			$$ = doubleRule;
		}
		| setting	{ $$ = $1; }
		;
optmoving
		: /* пусто */
		{
			double_rule* doubleRule = new double_rule;
				direction_const* directionConst1 = new direction_const;
				directionConst1->setDirCode(0);
				direction_const* directionConst2 = new direction_const;
				directionConst2->setDirCode(0);
			doubleRule->setAtLine(directionConst1, 0);
			doubleRule->setAtLine(directionConst2, 0);
			$$ = doubleRule;
		}
		| moving	{ $$ = $1; }
		;

optgoing: /* пусто */
		{
			going_const* goingConst = new going_const;
			goingConst->setStateIndex(indexCurrentState);
			$$ = goingConst;
		}
		| going	{ $$ = $1; }
		;

/* ----------------------------  setting ---------------------------- */

setting	: Value '=' cell_value ';'  { $$ = $3;  } ;

cell_value
		: value_expression
		{
			$$ = $1;
		}
		| double_line_value
		{
			$$ = $1;
		}
		;

double_line_value
		: '(' value_expression ',' value_expression ')'
		{
			double_rule* doubleRule = new double_rule;
			doubleRule->setAtLine($2, 0);
			doubleRule->setAtLine($4, 1);
			$$ = doubleRule;
		}
		;

value_expression
		: SYMBOL
		{
			symbol_value* symbolValue = new symbol_value;
			symbolValue->setValue($1);
			$$ = symbolValue;
		}
		| Empty
		{
			symbol_value* symbolValue = new symbol_value;
			symbolValue->setValue(0);
			$$ = symbolValue;
		}
		| IDENTIFIER
		{
			symbol_argument* symbolArg = new symbol_argument;
			int index = templatesTable[actualTemplateId]->getParameters()->indexOf($1);
			if(index != -1)
			{
				symbolArg->setArgIndex(index);
			}
			else
			{
				/* ошибка - идентификатор аргумента не определен */
			}
			$$ = symbolArg;
		}
		| First
		{
			symbol_copy* symbolCopy = new symbol_copy;
			symbolCopy->setLineSource(0);
			$$ = symbolCopy;
		}
		| Second
		{
			symbol_copy* symbolCopy = new symbol_copy;
			symbolCopy->setLineSource(1);
			$$ = symbolCopy;
		}
		| ELLIPSIS
		{
			symbol_const* symbolConst = new symbol_const;
			$$ = symbolConst;
		}
		;

/* ----------------------------  moving ----------------------------- */

moving	: Move dir_value ';' { $$ = $2; } ;

dir_value
		: single_dir
		{
			$$ = $1;
		}
		| double_dir 
		{
			$$ = $1;
		}
		;

double_dir
		: single_dir ',' single_dir
		{
			double_rule* doubleRule = new double_rule;
			doubleRule->setAtLine($1, 0);
			doubleRule->setAtLine($3, 1);
			$$ = doubleRule;
		}
		;

single_dir
		: const_dir
		{
			direction_const* directionConst = new direction_const;
			directionConst->setDirCode($1);
			$$ = directionConst;
		}
		| IDENTIFIER
		{
			direction_arg* directionArg = new direction_arg;
			int index = templatesTable[actualTemplateId]->getParameters()->indexOf($1);
			if(index != -1)
			{
				directionArg->setArgIndex(index);
			}
			else
			{
				/* ошибка - идентификатор аргумента не определен */
			}
			$$ = directionArg;
		}
		;

const_dir
		: Not	{ $$ = 0; }
		| Right { $$ = 1; }
		| Left	{ $$ = 2; }
		;

/* ----------------------------  going ------------------------------ */

going	: Goto going_expression ';'
		{
			//statement* Statement = new statement;
			//Statement->setNodeType(STATEMENT);
			//Statement->setFirst($2);
			//$$ = Statement;
			$$ = $2;
		}
		;

going_expression
		: IDENTIFIER
		{
			going_arg* goingArg = new going_arg;
			int index = templatesTable[actualTemplateId]->getParameters()->indexOf($1);
			if(index != -1)
			{
				goingArg->setArgIndex(index);
			}
			else
			{
				/* ошибка - идентификатор аргумента не определен */
			}
			$$ = goingArg;
		}
		| STATE_NUMBER
		{
			going_const* goingConst = new going_const;
			goingConst->setStateIndex($1);
			$$ = goingConst;
		}
		| Begin
		{
			going_const* goingConst = new going_const;
			goingConst->setStateIndex(0);
			$$ = goingConst;
		}
		| End
		{
			going_end* goingEnd = new going_end;
			$$ = goingEnd;
		}
		;


// -------------------------- value set ---------------------------- //

value_set	: symbol_value | values_list  { $$ = $1; } ;

symbol_value
			: ELLIPSIS
			{
				symbol_list* symbolList = new symbol_list;
				$$ = symbolList;
			}
			| Empty
			{
				symbol_list* symbolList = new symbol_list;
					symbol_condition* symbolCondition = new symbol_condition;
					symbolCondition->setValue(0);
				symbolList->addNodeLast(symbolCondition);
				$$ = symbolList;
			}
			| Alpha
			{
				/*symbol_list* symbolList = new symbol_list;
					symbol_condition* symbolCondition = new symbol_condition;
					symbolCondition->setValue('a');
				symbolList->addNodeLast(symbolCondition);
					symbolCondition = new symbol_condition;
					symbolCondition->setValue('b');
				symbolList->addNodeLast(symbolCondition);
					symbolCondition = new symbol_condition;
					symbolCondition->setValue('c');
				symbolList->addNodeLast(symbolCondition);*/
				$$ = turingMashine->getAlphaList();
			}
			| symbol_list	{ $$ = $1; }
			;

symbol_list	: SYMBOL
			{
				symbol_list* symbolList = new symbol_list;
					symbol_condition* symbolCondition = new symbol_condition;
					symbolCondition->setValue($1);
				symbolList->addNodeLast(symbolCondition);
				$$ = symbolList;
			}
			| symbol_list ',' SYMBOL
			{
				code_list* symbolList = $1;
					symbol_condition* symbolCondition = new symbol_condition;
					symbolCondition->setValue($3);
				symbolList->addNodeLast(symbolCondition);
				$$ = symbolList;
			}
			;

value_double
			: '(' symbol_value ';' symbol_value ')'
			{
				double_line_value* doubleLineValue = new double_line_value;
				doubleLineValue->setAtLine($2, 0);
				doubleLineValue->setAtLine($4, 1);
				$$ = doubleLineValue;
			}
			;

values_list
			: value_double
			{
				values_list* valuesList = new values_list;
				valuesList->addNodeLast($1);
				$$ = valuesList;
			}
			| values_list ',' value_double
			{
				code_list* valuesList = $1;
				valuesList->addNodeLast($3);
				$$ = valuesList;
			}
			;


%%