#include "morphology.h"


class ExprAST
{
private:
	string type;
public:
	virtual ~ExprAST() {}
	virtual Value* Codegen() = 0;
};

//real类型数节点
class NumberRealExprAST : public ExprAST
{
private:
	double num;
	string type;
public:
	NumberRealExprAST(double r) : num(r), type("real") {}
	virtual Value* Codegen();
};

//int类型数节点
class NumberIntExprAST : public ExprAST
{
private:
	int num;
	string type;
public:
	NumberIntExprAST(int i) : num(i), type("int") {}
	virtual Value* Codegen();
};

//变量节点//如何确定type
template<typename T>
class VariableExprAST : public ExprAST
{
private:
	string name;
	string type;
	T value;//此类型的数值
public:
	//处理变量引用的时候,可以不带类型
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
//参数节点，保存参数名和类型
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

//中间操作符节点
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

//保存函数名和参数的表达式列表
//函数调用
class CallExprAST : public ExprAST
{
private:
	string Callee;
	vector<ExprAST*> Args;
	string type;
public:
	//引用类型的原因？
	CallExprAST(string& c, vector<ExprAST*>& a) : Callee(c), Args(a), type("call") {}
	virtual Value* Codegen();
};

//函数原型
//将参数的arg形式改为了ast
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


//函数本身
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


