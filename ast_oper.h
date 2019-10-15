#include"ast.h" 

//函数运用比中缀运算符优先级更高，因此可以不用括号。

//假定变量必须指定类型
//假定函数必须带括号
// + - * 
// int 除法为div real为/ 取模 mod
//and andalso orelse not 
//if  then else
//local let in end
//val int real  type string bool char
//fun fn 
//infix  of op case do
//eof

static ExprAST *ParseExpression();


// + - * / 
static std::map<char,int> BinOpPrecedence;
//获取操作符的优先级  //但是操作符多了div mod 
//isascii判断内容是否在1-127之间
static int getPrecedence(){
	if(!isascii(CurTok))
		return -1;
	int prec=BinOpPrecedence[CurTok];
	if(prec<=0)return -1;
	return prec;
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
	ExprAST *result = new  NumberRealExprAST(realNum);
	getNextToken();
	return result;
}
//int   返回一个整型节点
static ExprAST *ParseIntExpr()
{
	ExprAST *result = new NumberIntExprAST(intNum);
	getNextToken();
	return result;
}

//处理变量声明
static ExprAST *ParseVariable(){
	getNextToken();//跳过val关键字
	if(CurTok!=tok_identifier){
		return Error("the identifier after val is wrong");
	}
	string variab=variableVal;
	getNextToken();
	if(CurTok!=':')
		return Error("the identifier must have a type");
	getNextToken();
	//还有字符串类型
	if(CurTok!=tok_real&&CurTok!=tok_int){
		return Error("the identifier type is wrong");
	}
	string typet=typeval;
	getNextToken();
	//经测试，似乎不能不赋值
	if(CurTok!='=')
		return Error("error : identifier has no val assignment");
	getNextToken();
	//似乎关键字和真正的立即数重合了
	if(CurTok==tok_real_num){
		if(typet.compare("real")!=0)
			return Error("type not matching");
		else{
			getNextToken();
			return new VariableExprAST<double>(variableVal,typet,realNum);
		}
	}else if(CurTok==tok_int_num){
		if(typet.compare("int")!=0)
			return Error("type not matching");
		else{
			getNextToken();
			return new  VariableExprAST<int>(variableVal,typet,intNum);
		}
	}
}

//处理变量引用和函数调用
//sml中的函数如果不带括号，多个参数的情况不能带逗号，以空格分隔
//暂时只处理带括号和逗号的情况
static ExprAST *ParseIdentifierExpr(){
	string IDname=IdentifierStr;//已获取标识符
	getNextToken();
	//只是简单的变量
	//可以有变量类型说明可以没有
	bool hastype=false;
	if(CurTok!='('){
		//查看是否有类型说明符
		if(CurTok==':'){
			hastype=true;
			getNextToken();
			string type=typeval;
			if(type.compare("int")!=0&&type.compare("real")!=0){
				return Error("no such type");
			}
			if("int"==type){
				return new VariableExprAST<int>(IDname,type);
			}else{
				return new VariableExprAST<double>(IDname,type);
			}
		}
		//如果没有类型说明符，如何推导?
		//函数上方
		return new VariableExprAST<int>(IDname);
	}
	//函数调用
	if(!hastype)   getNextToken();
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
		case ';':				getNextToken();break;
		case tok_identifier:return ParseIdentifierExpr();
		case tok_real:      return ParseRealExpr();
		case tok_int:       return ParseIntExpr();
		default:            return Error("unknown token when ecpecting an expression");
	}
}
//解析有序对列表
//进行优先级解析
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
		lhs=new BinaryExprAST(BinOp,lhs,rhs);
	}
}
//表达式的解析
static ExprAST *ParseExpression(){
	ExprAST* lhs=ParsePrimary();//目前返回的是int立即数或real立即数或标识符
	if(!lhs) return 0;
	return ParseBinOpRHS(0,lhs);
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



//解析函数原型
//返回的是函数名和函数参数的结点
static PrototypeAST* ParsePrototype(){
	if(CurTok!=tok_identifier) 
		return ErrorP("expected function name in prototype");
	string fnName=IdentifierStr;
	getNextToken();
	if(CurTok != '('){
		return ErrorP("expected '(' in prototype");
	} 
	vector<ParameterExprAST *> Args;
	getNextToken();
	while(CurTok!=')'){
		if(CurTok!=tok_identifier)
			return ErrorP("error,no identifier!");
		string IDname=IdentifierStr;
		getNextToken();
		if(CurTok!=':')
			return ErrorP("error,you should define this variable's type");
		getNextToken();
		if(CurTok!=tok_int&&CurTok!=tok_real)
			return ErrorP("no such type ");
		string type=typeval;
		if(type.compare("int")!=0&&type.compare("real")!=0){
			return ErrorP("error,no such type");
		}
		ParameterExprAST *arg;
		//这样的话如果参数有两种就会出错，vector中只会接受一种变量
		arg=new ParameterExprAST(IDname,type);
		Args.push_back(arg);
		getNextToken();
		if( CurTok!=','&&CurTok!=')'){
			return ErrorP("expected ')' in prototype");
		}
	}
	if(CurTok!=')'){
		return ErrorP("expected ')' in prototype");
	}
	getNextToken();
	return new PrototypeAST(IdentifierStr,Args);
}

//函数定义
static FunctionAST* ParseDefinition(){
	getNextToken();//eat def
	PrototypeAST* proto=ParsePrototype();
	if(!proto) return 0;
	//解析完函数前的全部的
	if(CurTok!='='){//由于ParsePrototype最后已经跳过了')'到了下一个，如果是函数定义必须为=
		return ErrorF("expected '=' in definition");
	}
	getNextToken();//eat '='
	if(ExprAST* E=ParseExpression())
		return new FunctionAST(proto,E);
	return 0;
}

//处理函数定义
static void HandleDefition(){
	if(ParseDefinition()){
		cout<<"Parsed a function definition\n";
	}else{
		getNextToken();
	}
}

//处理变量定义。
static void HandleVarDefition(){
	if(ParseVariable()){
		cout<<"Parsed a variable definition\n";
	}else{
		getNextToken();
	}
}
//处理直接写一个表达式的情况。
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
		PrototypeAST *Proto = new PrototypeAST(" ", vector<ParameterExprAST *>());
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
			case tok_fun:  HandleDefition();break;//处理函数定义
			case tok_val:  HandleVarDefition();break;//处理变量定义
			default:			HandleTopLevelExpression();
		}
	}
}


