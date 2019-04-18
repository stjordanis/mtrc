#pragma once

#include "conditions.h"

class compare_expression : public code_block
{
public:
	void setCompOperator(int comp_code) { this->comp_code = comp_code; }
	void setElseOffset(int else_offset) { this->else_offset = else_offset; }
	int getCodeLength() { return 4; }
	unsigned char* writeCode(unsigned char* ptr);
private:
	int comp_code;
	int else_offset;
};

class ifelse_block : public actions_block
{
public:
	void setCondition(compare_expression* cond) { this->cond = cond; }
	void setIfBlock(rule_action* block) { this->if_block = block; }
	void setElseBlock(rule_action* block) { this->else_block = block; }
	~ifelse_block()
	{
		if (else_block != NULL) delete else_block;
		delete cond;
		delete if_block;
	}
	void setEndOffset(int end_offset)
	{
		if (else_block)
		{
			else_block->setEndOffset(end_offset);
			if_block->setEndOffset(end_offset + else_block->getCodeLength());
		}
		else
		{
			if_block->setEndOffset(end_offset);
		}
		actions_block::setEndOffset(end_offset);
	}
	int getCodeLength() { return (cond->getCodeLength() + if_block->getCodeLength() + else_block->getCodeLength()); }
	unsigned char* writeCode(unsigned char* ptr);
	void writeGraph(FILE* gfile, int state_id)
	{
		if_block->writeGraph(gfile, state_id);
		if(else_block) else_block->writeGraph(gfile, state_id);
	}
private:
	compare_expression* cond;
	rule_action* if_block;
	rule_action* else_block;
};

class turing_rule : public actions_block
{
public:
	void setValues(conditions_list* conditionBlock) { this->conditionBlock = conditionBlock; }
	void setAction(actions_block* actionBlock) { this->actionBlock = actionBlock; }
	~turing_rule()
	{
		delete conditionBlock;
		delete actionBlock;
	}
	int getCodeLength() { return (conditionBlock->getCodeLength() + actionBlock->getCodeLength()); }
	unsigned char* writeCode(unsigned char* ptr);
	void setEndOffset(int end_offset)
	{
		actionBlock->setEndOffset(end_offset);
		actions_block::setEndOffset(end_offset);
	}
	void writeGraph(FILE* gfile, int state_id)
	{
		actionBlock->writeGraph(gfile, state_id);
	}
private:
	conditions_list* conditionBlock;
	actions_block* actionBlock;
};

class rules_list : public code_list
{
public:
	int getCodeLength()
	{
		int res = code_list::getCodeLength();
		return (res + 5); // res + 8
	}
	unsigned char* writeCode(unsigned char* ptr);
	void writeGraph(FILE* gfile, int state_id) const
	{
		for (code_block* node : list)
		{
			actions_block* act_ptr = dynamic_cast<actions_block*>(node);
			if (act_ptr) act_ptr->writeGraph(gfile, state_id);
		}
	}
};