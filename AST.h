#include "toy.h"

//语法树
//
//总父类
//需要有节点的类型
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
class VariableExprAST : public ExprAST
{
private:
    string name;
	 string type;
public:
    VariableExprAST(string s,string ty) : name(s),type(ty) {}
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
class CallExprAST : public ExprAST
{
private:
    string Callee;
    vector<ExprAST *> Args;
	 string type;
public:
    //引用类型的原因？
    CallExprAST(string &c, vector<ExprAST *> &a) : Callee(c), Args(a),type("call") {}
}

//函数接口
class PrototypeAST
{
private:
	string name;
	vector<string> Args;
	string type;
public:
    PrototypeAST(string &na, vector<string> &ar) : name(na), Args(ar) ,type("proto"){}
}

//函数本身
class FunctionAST
{
private:
    PrototypeAST *proto;
    ExprAST *body;
	 string type;
public:
    FunctionAST(PrototypeAST *pro, ExprAST *bo) : proto(pro), body(bo),type("fun") {}
}

