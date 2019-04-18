#include "rule_nodes.h"

unsigned char* compare_expression::writeCode(unsigned char* ptr)
{
	*ptr++ = 0x38;
	*ptr++ = 0xE0;
	*ptr++ = 0x70 + comp_code;
	*ptr++ = else_offset;
	return ptr;
}

unsigned char* ifelse_block::writeCode(unsigned char* ptr)
{
	cond->setElseOffset(if_block->getCodeLength());
	ptr = cond->writeCode(ptr);
	ptr = if_block->writeCode(ptr);
	if(else_block) ptr = else_block->writeCode(ptr);
	return ptr;
}

unsigned char* turing_rule::writeCode(unsigned char* ptr)
{
	conditionBlock->setElseIndex(actionBlock->getCodeLength());
	ptr = conditionBlock->writeCode(ptr);
	ptr = actionBlock->writeCode(ptr);
	return ptr;
}

unsigned char* rules_list::writeCode(unsigned char* ptr)
{
	int end_off = getCodeLength() /* + 5*/ /*- 3*/;
	//*ptr++ = 0x8B;
	//*ptr++ = 0x45;
	//*ptr++ = 0x08;
	for (code_block* node : list)
	{
		end_off -= node->getCodeLength();
		actions_block* act_ptr = dynamic_cast<actions_block*>(node);
		if (act_ptr) act_ptr->setEndOffset(end_off);
		ptr = node->writeCode(ptr);
	}
	*ptr++ = 0xB8;
	*ptr++ = 0;
	*ptr++ = 0;
	*ptr++ = 0x80;
	*ptr++ = 0;
	return ptr;
}