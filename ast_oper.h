#include"AST.h" 

//函数运用比中缀运算符优先级更高，因此可以不用括号。

	// + - * 
	// int 除法为div real为/ 取模 mod
	//and andalso orelse not 
	//if  then else
	//local let in end
	//val int real  type string bool char
	//fun fn 
	//infix  of op case do
	//eof
static double realNum;
static int intNum;
static int CurTok;
string string typeval;

// + - * / 
static map<char,int> BinOpPrecedence;
//获取操作符的优先级  //但是操作符多了div mod 
//isascii判断内容是否在1-127之间
static int getPrecedence(){
    if(!isascii(CurTok))
        return -1;
    int prec=BinOpPrecedence[CurTok];
    if(prec<=0)return -1;
    return prec;
}
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

//real    返回一个实数类型的节点
static ExprAST *ParseRealExpr()
{
    ExprAST *result = NumberRealExprAST(realNum);
    getNextToken();
    return result;
}
//int   返回一个整型节点
static ExprAST *ParseIntExpr()
{
    ExprAST *result = NumberIntExprAST(intNum);
    getNextToken();
    return result;
}
//暂时不考虑注释
//括号()的解析   注意其中的递归
//这里确定括号内部是表达式
static ExprAST *ParseParenExpr(){
    getNextToken();//忽略'('
    ExprAST *re=ParseExpression();
    if(!re) return 0;
	 //解析表达式后一定是')'
    if(CurTok!=')')
        return Error("expected ')'");
    getNextToken();
    return re;
}

//表达式的解析
static ExprAST *ParseExpression(){
    ExprAST* lhs=ParsePrimary();//目前返回的是int立即数或real立即数或标识符
    if(!lhs) return 0;
    return ParseBinOpRHS(0,lhs);
}

//解析有序对列表
static ExprAST* ParseBinOpRHS(int ExprPrec,ExprAST*lhs){
    while(1){
		 //获取下个操作符优先级
        int TokPrec=getPrecedence();
        if(TokPrec<ExprPrec)
            return lhs;
        int BinOp=CurTok;
        getNextToken();
        ExprAST *rhs=ParsePrimary();
        if(!rhs)return 0;
        int nextProcedence=getPrecedence();
 //如果出现有乘除的，直接将有乘除的部分合到一起作为右节点
        if(TokPrec<nextProcedence){
         rhs=ParseBinOpRHS(TokPrec+1,rhs);
         if(!rhs)   return 0;
        }
        lhs=new BinaryExprAST(Binop,lhs,rhs);
    }
}


//处理变量引用和函数调用
//判断到底是变量还是函数
//关键是可以不用'('和')'  当然现在这个只是解析表达式的辅助
//sml中的函数是必须有参数的
//注意函数调用如果是多个参数的情况则必须要带括号，否则可以不带括号。
//即可
//area r;
//funasda(r,x);
static ExprAST *ParseIdentifierExpr(){
    string IDname=IdentifierStr;//已获取标识符
    getNextToken();
	 //只是简单的变量
	 //暂时不考虑忽略括号
    if(CurTok!='('){
		 //查看是否有类型说明符
		 if(CurTok==':'){
			 getNextToken();
			 string type=typeval;
			 if(type!="int"||type!="real"){
				 return Error("no such type of value");
			 }
       	 return new VariableExprAST(IDname,type);
		 }
		 //如果没有类型说明符，如何推导?
		 return new VariableExprAST(IDname,"real");
	}
	 //函数调用
    getNextToken();
    vector<ExprAST*>Args;
    while(CurTok!=')'){//里面可能有类型说明
        ExprAST* arg=ParseExpression();
        if(!arg)return 0;
        Args.push_back(arg);
        if(CurTok==')')break;
        if(CurTok!=',')
            return Error("Excepted ')' or ',' in arguments");
        getNextToken();
    }
    getNextToken();//eat ')'
	 //返回的主要是函数名和参数名
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

//解析函数原型
//返回的是函数名和函数参数的结点
static ExprAST* ParsePrototype(){
    if(CurTok!=tok_identifier) 
        return ErrorP("expected function name in prototype");
    string fnName=IdentifierStr;
    getNextToken();
    if(CurTok != '('){
		 return ErrorP("expected '(' in prototype");
	 } 
    vector<string> Args;
	 bool isarg=false;
	 getNextToken();
    while(CurTok==tok_identifier||CurTok==','){
		 	   if(CurTok==','){
					if(!isarg)
					return	ErrorP("error,successive ','in args");
					isarg=false;
					getNextToken();
				}else{
        		Args.push_back(IdentifierStr);
		  		isarg=true;
		  		getNextToken();
				}
	 }
	 if(CurTok!=')'){
		 return ErrorP("expected ')' in prototype");
	 }
	 getNextToken();
    return new PrototypeAST(IdentifierStr,Args);
}

//函数定义
static ExprAST* ParseDefinition(){
    getNextToken();//eat def
    PrototypeAST* proto=ParsePrototype();
    if(!proto) return 0;
	 //解析完函数前的全部的
	 if(CurTok!='='){//由于ParsePrototype最后已经跳过了')'到了下一个，如果是函数定义必须为=
		 return ErrorF("expected '=' in definition");
	 }
    if(ExprAST* E=ParseExpression())
        return FunctionAST(proto,E);
    return 0;
}

static void HandleDefition(){
	if(ParseDefinition()){
		cout<<"Parsed a function definition\n";
	}else{
		getNextToken();
	}
}

static void HandleTopLevelExpression(){
	if(ParseExpression()){
		cout<<"Parsed a top-level expression\n";
	}else{
		getNextToken();
	}
}

//用于直接输入表达式求值
static FunctionAST *ParseTopLevelExpr() {
  if (ExprAST *E = ParseExpression()) {
    // Make an anonymous proto.
    PrototypeAST *Proto = new PrototypeAST("", std::vector<std::string>());
    return new FunctionAST(Proto, E);
  }
  return 0;
}

static void mainLoop(){
	while(1){
		cout<<"ready>"<<endl;
		switch(CurTok){
			case tok_eof:	return;
			case ';':		getNextToken();break;
			case tok_fun:  HandleDefition();break;
			default:			HandleTopLevelExpression();
		}
	}
}


int main(){
	BinopPrecedence['<'] = 10;
	BinopPrecedence['>'] = 10;
	BinopPrecedence['+'] = 20;
	BinopPrecedence['-'] = 20;
	BinopPrecedence['*'] = 40;  // highest.
 	BinOpPrecedence['/']	= 40;
 	cout<<"ready>\n";
  	getNextToken();
  	MainLoop();
  	return 0;
}
