#pragma once

#include "expressions.h"

class condition : virtual public code_block
{
public:
	virtual void setElseIndex(int else_index) { this->else_index = else_index; }
	virtual void setTrueOffset(int true_offset) { this->true_offset = true_offset; }
	condition() : true_offset(0), else_index(0) {} 
protected:
	int else_index;
	int true_offset;
};

class symbol_condition : public symbol_expression, public source_expression
{
public:
	int getCodeLength() { return /*(line_src == 1 ? */ 5 /* : 4)*/; }
	unsigned char* writeCode(unsigned char* ptr);
};

class conditions_list : public code_list, public condition
{
public:
	int getCodeLength() { return code_list::getCodeLength(); }
	unsigned char* writeCode(unsigned char* ptr);
};

class symbol_list : public conditions_list, public source_expression
{
public:
	void addNodeLast(code_block* node)
	{
		source_expression* ptr_src = dynamic_cast<source_expression*>(node);
		if (ptr_src) ptr_src->setLineSource(line_src);
		code_list::addNodeLast(node);
	}
	void setLineSource(int line_src)
	{
		source_expression::setLineSource(line_src);
		for (code_block* node : list)
		{
			source_expression* ptr_src = dynamic_cast<source_expression*>(node);
			if (ptr_src) ptr_src->setLineSource(line_src);
		}
	}
	int getCodeLength() { return conditions_list::getCodeLength(); }
	unsigned char* writeCode(unsigned char* ptr) { return conditions_list::writeCode(ptr); }
};


class double_line_value : public double_line_code, public condition
{
public:
	void setAtLine(code_block* node, int line)
	{
		source_expression* node_src = dynamic_cast<source_expression*>(node);
		if (node_src) node_src->setLineSource(line);
		double_line_code::setAtLine(node, line);
	}
	/*void setElseIndex(int else_index)
	{
		condition* cond_ptr;
		if (nodes[0] == NULL)
		{
			if (nodes[1] != NULL)
			{
				cond_ptr = dynamic_cast<condition*>(nodes[1]);
				if (cond_ptr) cond_ptr->setElseIndex(else_index);
			}
		}
		else if (nodes[1] == NULL)
		{
			condition* cond_ptr = dynamic_cast<condition*>(nodes[0]);
			if (cond_ptr) cond_ptr->setElseIndex(else_index);
		}
		else
		{
			condition* cond_ptr = dynamic_cast<condition*>(nodes[1]);
			if (cond_ptr) cond_ptr->setElseIndex(else_index);
			cond_ptr = dynamic_cast<condition*>(nodes[0]);
			if (cond_ptr) cond_ptr->setElseIndex(else_index + nodes[1]->getCodeLength());
		}
		condition::setElseIndex(else_index);
	}
	void setTrueOffset(int true_offset)
	{
		condition* cond_ptr;
		if (nodes[0] == NULL)
		{
			if (nodes[1] != NULL)
			{
				cond_ptr = dynamic_cast<condition*>(nodes[1]);
				if (cond_ptr) cond_ptr->setTrueOffset(true_offset);
			}
		}
		else if (nodes[1] == NULL)
		{
			condition* cond_ptr = dynamic_cast<condition*>(nodes[0]);
			if (cond_ptr) cond_ptr->setTrueOffset(true_offset);
		}
		else
		{
			condition* cond_ptr = dynamic_cast<condition*>(nodes[1]);
			if (cond_ptr) cond_ptr->setTrueOffset(true_offset);
			cond_ptr = dynamic_cast<condition*>(nodes[0]);
			if (cond_ptr) cond_ptr->setTrueOffset(0);
		}
		condition::setTrueOffset(true_offset);
	}*/
	int getCodeLength() { return double_line_code::getCodeLength(); }
	unsigned char* writeCode(unsigned char* ptr)
	{
		condition* cond_ptr = dynamic_cast<condition*>(nodes[1]);
		if (cond_ptr) cond_ptr->setElseIndex(else_index);
		cond_ptr = dynamic_cast<condition*>(nodes[0]);
		if (cond_ptr) cond_ptr->setElseIndex(else_index + nodes[1]->getCodeLength());
		cond_ptr = dynamic_cast<condition*>(nodes[1]);
		if (cond_ptr) cond_ptr->setTrueOffset(true_offset);
		cond_ptr = dynamic_cast<condition*>(nodes[0]);
		if (cond_ptr) cond_ptr->setTrueOffset(0);
		return double_line_code::writeCode(ptr);
	}
};

class values_list : public conditions_list
{
public:
	void addNodeLast(code_block* node)
	{
		condition* cond_ptr = dynamic_cast<condition*>(node);
		if (cond_ptr) cond_ptr->setElseIndex(0);
		code_list::addNodeLast(node);
	}
	unsigned char* writeCode(unsigned char* ptr)
	{
		int tr_off = true_offset + getCodeLength();
		condition* cond_ptr_end = NULL;
		for (code_block* node : list)
		{
			condition* cond_ptr = dynamic_cast<condition*>(node);
			if (cond_ptr)
			{
				tr_off -= node->getCodeLength();
				cond_ptr->setElseIndex(0);
				cond_ptr->setTrueOffset(tr_off);
				cond_ptr_end = cond_ptr;
			}
		}
		cond_ptr_end->setElseIndex(else_index);
		return conditions_list::writeCode(ptr);
	}
};

/* ------------------------- if-else conditions -------------------------- */

