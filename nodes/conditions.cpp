#include "conditions.h"

unsigned char* symbol_condition::writeCode(unsigned char* ptr)
{
	if (line_src == 1)
	{
		*ptr++ = 0x80;
		*ptr++ = 0xFC;
	}
	else
	{
		*ptr++ = 0x90;
		*ptr++ = 0x3C;
	}
	*ptr++ = value;
	*ptr++ = 0;
	*ptr++ = 0;
	return ptr;
}


unsigned char* conditions_list::writeCode(unsigned char* ptr)
{
	unsigned char disp = getCodeLength() + true_offset;
	for (code_block* node : list)
	{
		ptr = node->writeCode(ptr);
		disp -= node->getCodeLength();
		*(ptr - 2) = 0x74;
		*(ptr - 1) = disp;
	}
	if (true_offset == 0 && !list.empty())
	{
		*(ptr - 2) = 0x75;
		*(ptr - 1) = else_index;
	}
	return ptr;
}