#include "morphology.h"


class ExprAST
{
private:
	string type;
public:
	virtual ~ExprAST() {}
	virtual Value* Codegen() = 0;
};

//real�������ڵ�
class NumberRealExprAST : public ExprAST
{
private:
	double num;
	string type;
public:
	NumberRealExprAST(double r) : num(r), type("real") {}
	virtual Value* Codegen();
};

//int�������ڵ�
class NumberIntExprAST : public ExprAST
{
private:
	int num;
	string type;
public:
	NumberIntExprAST(int i) : num(i), type("int") {}
	virtual Value* Codegen();
};

//�����ڵ�//���ȷ��type
template<typename T>
class VariableExprAST : public ExprAST
{
private:
	string name;
	string type;
	T value;//�����͵���ֵ
public:
	//����������õ�ʱ��,���Բ�������
	VariableExprAST(string n) :name(n), type("") {};
	VariableExprAST(string s, string ty) : name(s), type(ty), value(0) {}
	VariableExprAST(string s, string ty, T val) :name(s), type(ty), value(val) {}
	void setType(string t) {
		type = t;
	}
	void setValue(T v) {
		value = v;
	};
	//code generate
	virtual Value* Codegen() {
		Value* V = NamedValues[name];
		if (!V)
			return MyErrorV("Unknown variable name");
		return V;
	}
};

/*
//�����ڵ㣬���������������
class ParameterExprAST : public ExprAST
{
private:
	string name;
	string type;
public:
	string getName() { return name; }
	ParameterExprAST(string n, string t) :name(n), type(t) {}
	virtual Value* Codegen();
};
*/

//�м�������ڵ�
class BinaryExprAST : public ExprAST
{
private:
	char Op;
	ExprAST* LHS, * RHS;
	string type;
public:
	BinaryExprAST(char op, ExprAST* l, ExprAST* r) : Op(op), LHS(l), RHS(r), type("op") {}
	virtual Value* Codegen();
};

//���溯�����Ͳ����ı��ʽ�б�
//��������
class CallExprAST : public ExprAST
{
private:
	string Callee;
	vector<ExprAST*> Args;
	string type;
public:
	//�������͵�ԭ��
	CallExprAST(string& c, vector<ExprAST*>& a) : Callee(c), Args(a), type("call") {}
	virtual Value* Codegen();
};

//����ԭ��
//��������arg��ʽ��Ϊ��ast
class PrototypeAST{
private:
	string name;
	vector<string > Args;
	string type;
public:
	PrototypeAST(const string& na, const vector<string >& ar) : name(na), Args(ar), type("proto") {}
	Function* Codegen();
	const std::string& getName() const { return name; }
};


//��������
class FunctionAST
{
private:
	PrototypeAST* proto;
	ExprAST* body;
	string type;
public:
	FunctionAST(PrototypeAST* pro, ExprAST* bo) : proto(pro), body(bo), type("fun") {}
	Function* Codegen();
};


