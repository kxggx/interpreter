#include "morphology.h"


class ExprAST
{
private:
	 string type;
public:
    virtual ~ExprAST() {}
};

//real类型数节点
class NumberRealExprAST : public ExprAST
{
private:
    double num;
	 string type;
public:
    NumberRealExprAST(double r) : num(r),type("real") {}
};

//int类型数节点
class NumberIntExprAST : public ExprAST
{
private:
    int num;
	 string type;
public:
    NumberIntExprAST(int i) : num(i),type("int") {}
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
	 VariableExprAST(string n):name(n),type(""){};
    VariableExprAST(string s,string ty) : name(s),type(ty),value(0) {}
	 VariableExprAST(string s,string ty,T val):name(s),type(ty),value(val){}
	 void setType(string t){
		 type=t;
	 }
	 void setValue(T v){
		 value=v;
	 };
};

//参数节点，保存参数名和类型
class ParameterExprAST : public ExprAST
{
private:
    string name;
	 string type;
public:
	 ParameterExprAST(string n,string t):name(n),type(t){}
};
//中间操作符节点
class BinaryExprAST : public ExprAST
{
private:
    char Op;
    ExprAST *LHS, *RHS;
	 string type;
public:
    BinaryExprAST(char op, ExprAST *l, ExprAST *r) : Op(op), LHS(l), RHS(r),type("op") {}
};

//保存函数名和参数的表达式列表
//函数调用
class CallExprAST : public ExprAST
{
private:
    string Callee;
    vector<ExprAST *> Args;
	 string type;
public:
    //引用类型的原因？
    CallExprAST(string &c, vector<ExprAST *> &a) : Callee(c), Args(a),type("call") {}
};

//函数原型
//将参数的arg形式改为了ast
class PrototypeAST
{
private:
	string name;
	vector<ParameterExprAST* > Args;
	string type;
public:
    PrototypeAST(const string &na,const vector<ParameterExprAST* >  &ar) : name(na), Args(ar) ,type("proto"){}
};

//函数本身
class FunctionAST
{
private:
    PrototypeAST *proto;
    ExprAST *body;
	 string type;
public:
    FunctionAST(PrototypeAST *pro, ExprAST *bo) : proto(pro), body(bo),type("fun") {}
};

