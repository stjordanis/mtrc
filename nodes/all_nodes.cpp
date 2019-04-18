#include "all_nodes.h"

unsigned char* template_call::writeCode(unsigned char* ptr)
{
	ptr = args->writeCode(ptr);
	int rel = templUnit->getOffset(ptr) - 5;
	*ptr++ = 0xE8;
	*ptr++ = (unsigned char)rel;
	rel >>= 8;
	*ptr++ = (unsigned char)rel;
	rel >>= 8;
	*ptr++ = (unsigned char)rel;
	rel >>= 8;
	*ptr++ = (unsigned char)rel;
	return ptr;
}

unsigned char* translation_unit::writeCode(unsigned char* ptr)
{
	if (rules) ptr = rules->writeCode(ptr);
	if (inherit) ptr = inherit->writeCode(ptr);
	return ptr;
}

unsigned char* state_unit::writeCode(unsigned char* ptr)
{
	address = ptr;
	//*ptr++ = 0x55;
	//*ptr++ = 0x89;
	//*ptr++ = 0xE5;
	ptr = translation_unit::writeCode(ptr);
	//*ptr++ = 0xC9;
	//*ptr++ = 0xC2;
	//*ptr++ = 0x04;
	//*ptr++ = 0x00;
	*ptr++ = 0xC3;
	return ptr;
}

unsigned char* template_unit::writeCode(unsigned char* ptr)
{
	address = ptr;
	//*ptr++ = 0x55;
	//*ptr++ = 0x89;
	//*ptr++ = 0xE5;
	//*ptr++ = 0x51;
	ptr = translation_unit::writeCode(ptr);
	//*ptr++ = 0x59;
	//*ptr++ = 0xC9;
	*ptr++ = 0xC2;
	unsigned short n_args = (params->getCount() * 4)/* + 4*/;
	*ptr++ = (unsigned char)n_args;
	n_args >>= 4;
	*ptr++ = (unsigned char)n_args;
	return ptr;
}

//unsigned char* turing_mashine::writeCode(unsigned char* ptr)
//{
//	units->setStartPtr(ptr);
//	ptr = units->writeCode(ptr);
//	return ptr;
//}

symbol_list* turing_mashine::getAlphaList() const
{
	return new alpha_symbol_list(this);
}


/*	dispatch_table
	code generation example
------------- asm ------------
_get_rule:
000000F9: 55                 push        ebp
000000FA: 89 E5              mov         ebp,esp
000000FC: 8B 45 08           mov         eax,dword ptr [ebp+8]
000000FF: 48                 dec         eax
00000100: 8D 04 C5 0C 01 00  lea         eax,.text[eax*8+10Ch]
00
00000107: FF 75 0C           push        dword ptr [ebp+0Ch]
0000010A: FF E0              jmp         eax
0000010C: 90                 nop
0000010D: E8 EE FE FF FF     call        00000000
00000112: EB 20              jmp         00000134
00000114: 90                 nop
00000115: E8 67 FF FF FF     call        00000081
0000011A: EB 18              jmp         00000134
0000011C: 90                 nop
0000011D: E8 92 FF FF FF     call        000000B4
00000122: EB 10              jmp         00000134
00000124: 90                 nop
00000125: E8 68 FF FF FF     call        00000092
0000012A: EB 08              jmp         00000134
0000012C: 90                 nop
0000012D: E8 71 FF FF FF     call        000000A3
00000132: 90                 nop
00000133: 90                 nop
00000134: C9                 leave
00000135: C3                 ret
--------- new example -------------
_get_rule:
0000001C: 49                 dec         ecx
0000001D: 8D 0C CD 26 00 00  lea         ecx,.text[ecx*8+26h]
00
00000024: FF E1              jmp         ecx
00000026: E8 D4 FF FF FF     call        00000000
0000002B: 90                 nop
0000002C: 90                 nop
0000002D: C3                 ret
----------------------------------
*/


unsigned char* dispatch_table::writeCode(unsigned char* ptr)
{
	address = ptr;
	*ptr++ = 0x49;	// dec	ecx
	*ptr++ = 0x8D;
	*ptr++ = 0x0C;
	*ptr++ = 0xCD;	// lea	ecx, .begin_swintch[ecx*8]
	int disp = ((int)(ptr - start_ptr) + 6);
	relocation_ptr = ptr;
	*ptr++ = (unsigned char)disp;
	disp >>= 8;
	*ptr++ = (unsigned char)disp;
	disp >>= 8;
	*ptr++ = (unsigned char)disp;
	disp >>= 8;
	*ptr++ = (unsigned char)disp;
	*ptr++ = 0xFF;
	*ptr++ = 0xE1;	//	jmp		ecx
	for (int i = 0; i < count; i++)
	{
		if (sub_units[i] != NULL)
		{
			disp = sub_units[i]->getOffset(ptr) - 5;
			*ptr++ = 0xE8;
			*ptr++ = (unsigned char)disp;
			disp >>= 8;
			*ptr++ = (unsigned char)disp;
			disp >>= 8;
			*ptr++ = (unsigned char)disp;
			disp >>= 8;
			*ptr++ = (unsigned char)disp;
		}
		else
		{
			for (int j = 0; j < 5; j++)
				*ptr++ = 0x90;
		}
		if (i < count - 1)
		{
			*ptr++ = 0xEB;
			*ptr++ = (count - i - 1) * 8;
		}
		else
		{
			*ptr++ = 0x90;
		}
		*ptr++ = 0x90;
	}
	*ptr++ = 0xC3;
	return ptr;
}

/*
old code

unsigned char* dispatch_table::writeCode(unsigned char* ptr)
{
	address = ptr;
	*ptr++ = 0x55;	// push	ebp
	*ptr++ = 0x89;	
	*ptr++ = 0xE5;	// mov	ebp,esp
	*ptr++ = 0x8B;
	*ptr++ = 0x45;
	*ptr++ = 0x08;	// mov	eax,dword ptr [ebp+8]
	*ptr++ = 0x48;	// dec	eax
	*ptr++ = 0x8D;
	*ptr++ = 0x04;
	*ptr++ = 0xC5;	// lea	eax, .begin_swintch[eax*8]
	int disp = ((int)(ptr - start_ptr) + 9);
	relocation_ptr = ptr;
	*ptr++ = (unsigned char)disp;
	disp >>= 8;
	*ptr++ = (unsigned char)disp;
	disp >>= 8;
	*ptr++ = (unsigned char)disp;
	disp >>= 8;
	*ptr++ = (unsigned char)disp;
	*ptr++ = 0xFF; 
	*ptr++ = 0x75; 
	*ptr++ = 0x0C;	//	push	dword ptr[ebp + 0Ch]
	*ptr++ = 0xFF;
	*ptr++ = 0xE0;	//	jmp		eax
	for (int i = 0; i < count; i++)
	{
		if (sub_units[i] != NULL)
		{
			disp = sub_units[i]->getOffset(ptr) - 5;
			*ptr++ = 0xE8;
			*ptr++ = (unsigned char)disp;
			disp >>= 8;
			*ptr++ = (unsigned char)disp;
			disp >>= 8;
			*ptr++ = (unsigned char)disp;
			disp >>= 8;
			*ptr++ = (unsigned char)disp;
		}
		else
		{
			for (int j = 0; j < 5; j++)
				*ptr++ = 0x90;
		}
		if (i < count - 1)
		{
			*ptr++ = 0xEB;
			*ptr++ = (count - i - 1) * 8;
		}
		else
		{
			*ptr++ = 0x90;
		}
		*ptr++ = 0x90;
	}
	*ptr++ = 0xC9;
	*ptr++ = 0xC3;
	return ptr;
}

*/