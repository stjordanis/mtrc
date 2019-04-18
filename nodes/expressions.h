#pragma once

#include <stdio.h>
#include "turing.h"

/* ----------------------- expressions ------------------------ */

class destination_expression : virtual public code_block
{
public:
	void setLineDestination(int line_dst) { this->line_dst = line_dst; }
	destination_expression() : line_dst(0) {}
protected:
	int line_dst;
};

class source_expression : virtual public code_block
{
public:
	virtual void setLineSource(int line_src) { this->line_src = line_src; }
	source_expression() : line_src(0) {}
protected:
	int line_src;
};

class rule_expression : virtual public code_block
{
public:
	int getCodeLength() { return 0; }
	unsigned char* writeCode(unsigned char* ptr) { return ptr; }
	virtual unsigned long getRuleInfo() = 0;
};


class symbol_expression : virtual public code_block
{
public:
	void setValue(unsigned char value) { this->value = value; }
protected:
	unsigned char value;
};

class direction_expression : virtual public code_block
{
public:
	void setDirCode(int code) { this->code = code; }
protected:
	int code;
};

class going_expression : virtual public code_block
{
public:
	void setStateIndex(int index) { this->index = index; }
protected:
	int index;
};


/* ----------------------- argument expressions ------------------ */

class argument_expression : public rule_expression
{
public:
	void setArgIndex(int index) { this->arg_index = index; }
	unsigned long getRuleInfo() { return 0; }
	int getCodeLength() { return 4; }
	unsigned char* writeCode(unsigned char* ptr);
private:
	int arg_index;
};

/* ----------------------- symbol expressions -------------------- */

class symbol_value : /*public rule_expression, */public destination_expression, public symbol_expression
{
public:
	/*unsigned long getRuleInfo()
	{
		return ((unsigned long)value) << (line_dst * 8);
	}*/
	int getCodeLength() { return 2; }
	unsigned char* writeCode(unsigned char* ptr);
};

class symbol_copy : public source_expression, public destination_expression
{
public:
	int getCodeLength() { return 2 /*5*/; }
	unsigned char* writeCode(unsigned char* ptr);
};

class symbol_argument : public argument_expression, public destination_expression
{
public:
	int getCodeLength() { return 40; }
	unsigned char* writeCode(unsigned char* ptr);
};

class symbol_const : public rule_expression, public destination_expression
{
public:
	unsigned long getRuleInfo()
	{
		return 0x00100000 << line_dst;
	}
	int getCodeLength() { return rule_expression::getCodeLength(); }
	unsigned char* writeCode(unsigned char* ptr) { return rule_expression::writeCode(ptr); }
};

/* ----------------------- direction expressions -------------------- */


class direction_const : public rule_expression, public destination_expression, public direction_expression
{
public:
	unsigned long getRuleInfo()
	{
		return ((unsigned long)((line_dst == 1) ? (code * 3) : (code)) << 16);
	}
	int getCodeLength() { return rule_expression::getCodeLength(); }
	unsigned char* writeCode(unsigned char* ptr) { return rule_expression::writeCode(ptr); }
};

class direction_arg : public argument_expression, public destination_expression
{
public:
	int getCodeLength() { return 8 + line_dst * 3; }
	unsigned char* writeCode(unsigned char* ptr);
};

/* -------------------------- going expressions ---------------------- */

class going_const : public rule_expression, public going_expression
{
public:
	unsigned long getRuleInfo() { return ((unsigned long)index) << 24; }
	int getCodeLength() { return rule_expression::getCodeLength(); }
	unsigned char* writeCode(unsigned char* ptr) { return rule_expression::writeCode(ptr); }
	void writeGraph(FILE* gfile, int state_id)
	{
		fprintf_s(gfile, "  %d -> %d \n", state_id, index);
	}
};

class going_arg : public argument_expression
{
public:
	int getCodeLength() { return 16 + argument_expression::getCodeLength(); }
	unsigned char* writeCode(unsigned char* ptr);
};

class going_end : public rule_expression
{
public:
	unsigned long getRuleInfo() { return 0x00400000; }
	void writeGraph(FILE* gfile, int state_id)
	{
		fprintf_s(gfile, "  z [ label = < q<sub>z</sub> > ]\n");
		fprintf_s(gfile, "  %d -> z ", state_id);
	}
};


/* ----------------------- statements ------------------------ */
//
//class statement : public rule_expression
//{
//public:
//	void setFirst(destination_expression* fst)
//	{
//		first = fst;
//		first->setLineDestination(0);
//	}
//	statement() : rule_expression() { first = NULL; }
//	~statement() { delete first; }
//	unsigned long getRuleInfo()
//	{
//		if (first == NULL) return 0;
//		return ((rule_expression*)first)->getRuleInfo();
//	}
//	int getCodeLength() const
//	{
//		if (first == NULL) return 0;
//		return first->getCodeLength();
//	}
//	unsigned char* writeCode(unsigned char* ptr)
//	{
//		if (first == NULL) return ptr;
//		return first->writeCode(ptr);
//	}
//private:
//	destination_expression* first;
//};

class double_line_code : virtual public code_block
{
public:
	virtual void setAtLine(code_block* node, int line)
	{
		nodes[line] = node;
	}
	double_line_code() { nodes[0] = NULL; nodes[1] = NULL; }
	~double_line_code() { delete nodes[0]; delete nodes[1]; }
	int getCodeLength()
	{
		if (nodes[1] == NULL) return nodes[0]->getCodeLength();
		return nodes[0]->getCodeLength() + nodes[1]->getCodeLength();
	}
	unsigned char* writeCode(unsigned char* ptr)
	{
		if (nodes[1] == NULL) return nodes[0]->writeCode(ptr);
		return nodes[1]->writeCode(nodes[0]->writeCode(ptr));
	}
protected:
	code_block* nodes[2];
};

class double_rule : public double_line_code, public rule_expression
{
public:
	void setAtLine(code_block* node, int line)
	{
		destination_expression* node_dst = dynamic_cast<destination_expression*>(node);
		if (node_dst) node_dst->setLineDestination(line);
		double_line_code::setAtLine(node, line);
	}
	unsigned long getRuleInfo()
	{
		rule_expression* node_fst = dynamic_cast<rule_expression*>(nodes[0]);
		rule_expression* node_sec = dynamic_cast<rule_expression*>(nodes[1]);
		unsigned long res = 0;
		if (node_fst) res += node_fst->getRuleInfo();
		if (node_sec) res += node_sec->getRuleInfo();
		return res;
	}
	int getCodeLength() { return double_line_code::getCodeLength(); }
	unsigned char* writeCode(unsigned char* ptr) { return double_line_code::writeCode(ptr); }
};


/* ----------------------- action ---------------------------- */

class actions_block : virtual public code_block
{
public:
	virtual void setEndOffset(int end_offset) { this->end_offset = end_offset; }
	int getCodeLength() { return 2; }
	unsigned char* writeCode(unsigned char* ptr);
	virtual void writeGraph(FILE* gfile, int state_id) = 0;
private:
	int end_offset;
};

class rule_action : public actions_block
{
public:
	void setSetting(code_block* stmt) { this->settingStmt = stmt; }
	void setMoving(code_block* stmt) { this->movingStmt = stmt; }
	void setGoing(code_block* stmt) { this->goingStmt = stmt; }
	rule_action() : settingStmt(NULL), movingStmt(NULL), goingStmt(NULL) {}
	~rule_action()
	{
		delete settingStmt;
		delete movingStmt;
		delete goingStmt;
	}
	int getCodeLength()
	{
		return (5 + (settingStmt ? settingStmt->getCodeLength() : 0) +
			(movingStmt? movingStmt->getCodeLength() : 0) +
			(goingStmt? goingStmt->getCodeLength() : 0) +
			actions_block::getCodeLength());
	}
	unsigned char* writeCode(unsigned char* ptr);
	void writeGraph(FILE* gfile, int state_id)
	{
		going_const* gcnst_ptr = dynamic_cast<going_const*>(goingStmt);
		if (gcnst_ptr) gcnst_ptr->writeGraph(gfile, state_id);

		going_end* gend_ptr = dynamic_cast<going_end*>(goingStmt);
		if (gend_ptr) gend_ptr->writeGraph(gfile, state_id);
	}
private:
	code_block* settingStmt;
	code_block* movingStmt;
	code_block* goingStmt;
};


