#include "toy.h"

static double realNum;
static int intNum;

static map<char,int> BinOpPrecedence;
 
static int getPrecedence(){
    if(!isascii(CurTok))
        return -1;
    int prec=BinOpPrecedence[CurTok];
    if(prec<=0)return -1;
    return prec;
}
//总父类
class ExprAST
{
public:
    virtual ~ExprAST() {}
};

//real类型数
class NumberRealExprAST : public ExprAST
{
private:
    double num;

public:
    NumberRealExprAST(double r) : num(r) {}
};

//int类型数
class NumberIntExprAST : public ExprAST
{
private:
    int num;

public:
    NumberIntExprAST(int i) : num(i) {}
};

//变量
class VariableExprAST : public ExprAST
{
private:
    string name;

public:
    VariableExprAST(string s) : name(s) {}
};

//
class BinaryExprAST : public ExprAST
{
private:
    char Op;
    ExprAST *LHS, *RHS;

public:
    BinaryExprAST(char op, ExprAST *l, ExprAST *r) : Op(op), LHS(l), RHS(r) {}
};

class CallExprAST : public ExprAST
{
private:
    string Callee;
    vector<ExprAST *> Args;

public:
    //引用类型的原因？
    CallExprAST(string &c, vector<ExprAST *> &a) : Callee(c), Args(a) {}
}

class PrototypeAST
{
private:
    string name;
    vector<string> Args;

public:
    PrototypeAST(string &na, vector<string> &ar) : name(na), Args(ar) {}
}

class FunctionAST
{
private:
    PrototypeAST *proto;
    ExprAST *body;
public:
    FunctionAST(PrototypeAST *pro, ExprAST *bo) : proto(pro), body(bo) {}
}

static int CurTok;
static int getNextToken()
{
    return CurTok = gettok();
}

ExprAST *Error(const char *str)
{
    cout << "Error:" << str << endl;
    return 0;
}
PrototypeAST *ErrorP(const char *str)
{
    Error(str);
    return 0;
}

FunctionAST *ErrorF(const char *str)
{
    Error(str);
    return 0;
}

//real
static ExprAST *ParseRealExpr()
{
    ExprAST *result = NumberRealExprAST(realNum);
    getNextToken();
    return result;
}

//int
static ExprAST *ParseIntExpr()
{
    ExprAST *result = NumberIntExprAST(intNum);
    getNextToken();
    return result;
}

//括号()的解析  
static ExprAST *ParseParenExpr(){
    getNextToken();//忽略'('
    ExprAST *re=ParseExpression();
    if(!re)return 0;
    if(CurTok!=')')
        return Error("expected ')'");
    getNextToken();
    return re;
}

//表达式的解析
static ExprAST *ParseExpression(){
    ExprAST* lhs=ParsePrimary();
    if(!lhs) return 0;
    return ParseBinOpRHS(0,lhs);
}


static ExprAST* ParseBinOpRHS(int ExprPrec,ExprAST*lhs){
    while(1){
        int TokPrec=getPrecedence();
        if(TokPrec<ExprPrec)
            return lhs;
        int BinOp=CurTok;
        getNextToken();
        ExprAST *rhs=ParsePrimary();
        if(!rhs)return 0;
        int nextProcedence=getPrecedence();
        if(TokPrec<nextProcedence){
         rhs=ParseBinOpRHS(TokPrec+1,rhs);
         if(!rhs)   return 0;
        }
        lhs=new BinaryExprAST(Binop,lhs,rhs);
    }
    return;
}

//处理变量引用和h函数调用

static ExprAST *ParseIdentifierExpr(){
    string IDname=IdentifierStr;
    getNextToken();
    if(CurTok!='(')//只是简单的变量
        return new VariableExprAST(IDname);
    getNextToken();
    vector<ExprAST*>Args;
    while(CurTok!=')'){
        ExprAST* arg=ParseExpression();
        if(!arg)return 0;
        Args.push_back(arg);
        if(CurTok==')')break;
        if(CurTok!=',')
            return Error("Excepted ')' or ',' in arguments");
        getNextToken();
    }
    getNextToken();//eat ')'
    return new CallExprAST(IDname,Args);
}

//主表达式
static ExprAST* ParsePrimary(){
    switch(CurTok){
        case tok_identifier:return ParseIdentifierExpr();
        case tok_real:      return ParseRealExpr();
        case tok_int:       return ParseIntExpr();
        default:            return Error("unknown token when ecpecting an expression");
    }
}

static ExprAST* ParsePrototype(){
    if(CurTok!=tok_identifier) 
        return ErrorP("expected function name in prototype");
    string fnName=IdentifierStr;
    getNextToken();
    if(CurTok!='(')
        return ErrorP("expected '(' in prototype");
    vector<string> Args;
    //应该要考虑到,吧
    while(getNextToken()==tok_identifier)
        Args.push_back(IdentifierStr);
    if(CurTok!=')')
        return ErrorP("expected ')" in prototype);
    getNextToken();
    return new PrototypeAST(IdentifierStr,Args);
}

static ExprAST* ParseDefinition(){
    getNextToken();//eat def
    PrototypeAST* proto=ParsePrototype();
    if(!proto) return 0;
    if(ExprAST* E=ParseExpression())
        return FunctionAST(proto,E);
    return 0;
}

static FunctionAST *ParseTopLevelExpr() {
  if (ExprAST *E = ParseExpression()) {
    // Make an anonymous proto.
    PrototypeAST *Proto = new PrototypeAST("", std::vector<std::string>());
    return new FunctionAST(Proto, E);
  }
  return 0;
}
