#pragma once

#include <list>

struct rule_info
{
	unsigned char a_fst : 8;
	unsigned char a_sec : 8;
	unsigned char dir : 4;
	unsigned char a_fst_const : 1;
	unsigned char a_sec_const : 1;
	unsigned char end_flag : 1;
	unsigned char owerflow_flag : 1;
	unsigned char q_next : 8;
};

class code_block
{
public:
	virtual int getCodeLength() = 0;
	virtual unsigned char* writeCode(unsigned char* ptr) = 0;
	virtual ~code_block() {};
};

class code_list : virtual public code_block
{
public:
	virtual void addNodeLast(code_block* node) { list.push_back(node); }
	virtual ~code_list();
	int getCodeLength();
	unsigned char* writeCode(unsigned char* ptr);
protected:
	std::list<code_block*> list;
};

