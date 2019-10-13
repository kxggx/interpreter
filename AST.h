#include "toy.h"

//语法树
//
//总父类
class ExprAST
{
public:
    virtual ~ExprAST() {}
};

//real类型数节点
class NumberRealExprAST : public ExprAST
{
private:
    double num;

public:
    NumberRealExprAST(double r) : num(r) {}
};

//int类型数节点
class NumberIntExprAST : public ExprAST
{
private:
    int num;
public:
    NumberIntExprAST(int i) : num(i) {}
};

//变量节点
class VariableExprAST : public ExprAST
{
private:
    string name;

public:
    VariableExprAST(string s) : name(s) {}
};

//中间操作符节点
class BinaryExprAST : public ExprAST
{
private:
    char Op;
    ExprAST *LHS, *RHS;

public:
    BinaryExprAST(char op, ExprAST *l, ExprAST *r) : Op(op), LHS(l), RHS(r) {}
};

//保存函数名和参数的表达式列表
class CallExprAST : public ExprAST
{
private:
    string Callee;
    vector<ExprAST *> Args;

public:
    //引用类型的原因？
    CallExprAST(string &c, vector<ExprAST *> &a) : Callee(c), Args(a) {}
}

//函数接口
class PrototypeAST
{
private:
    string name;
    vector<string> Args;

public:
    PrototypeAST(string &na, vector<string> &ar) : name(na), Args(ar) {}
}

//函数本身
class FunctionAST
{
private:
    PrototypeAST *proto;
    ExprAST *body;
public:
    FunctionAST(PrototypeAST *pro, ExprAST *bo) : proto(pro), body(bo) {}
}

