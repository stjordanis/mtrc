#include "parameters.h"

unsigned char* going_argument_value::writeCode(unsigned char* ptr)
{
	int value = this->index;
	*ptr++ = 0x68;
	*ptr++ = (unsigned char)value;
	value >>= 8;
	*ptr++ = (unsigned char)value;
	value >>= 8;
	*ptr++ = (unsigned char)value;
	value >>= 8;
	*ptr++ = (unsigned char)value;
	return ptr;
}

unsigned char* direction_argument_value::writeCode(unsigned char* ptr)
{
	*ptr++ = 0x6A;
	*ptr++ = (unsigned char)this->code;
	return ptr;
}

unsigned char* symbol_argument_value::writeCode(unsigned char* ptr)
{
	*ptr++ = 0x6A;
	*ptr++ = this->value;
	return ptr;
}

unsigned char* argument_list::writeCode(unsigned char* ptr)
{
	for (code_block* node : list)
	{
		ptr = node->writeCode(ptr);
	}
	//*ptr++ = 0xFF;
	//*ptr++ = 0x75;
	//*ptr++ = 0x08;
	return ptr;
}