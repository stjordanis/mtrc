#include "turing.h"

code_list::~code_list()
{
	for (code_block* node : list)
	{
		delete node;
	}
}

int code_list::getCodeLength()
{
	int res = 0;
	for (code_block* node : list)
	{
		res += node->getCodeLength();
	}
	return res;
}

unsigned char* code_list::writeCode(unsigned char* ptr)
{
	for (code_block* node : list)
	{
		ptr = node->writeCode(ptr);
	}
	return ptr;
}