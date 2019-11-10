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

static ExprAST* ParseExpression();


// + - * / 
static std::map<char, int> BinOpPrecedence;
//获取操作符的优先级  //但是操作符多了div mod 
//isascii判断内容是否在1-127之间
static int getPrecedence() {
	if (!isascii(CurTok))
		return -1;
	int prec = BinOpPrecedence[CurTok];
	if (prec <= 0)return -1;
	return prec;
}


ExprAST* MyError(const char* str)
{
	cout << "Error:" << str << endl;
	return 0;
}

PrototypeAST* ErrorP(const char* str)
{
	MyError(str);
	return 0;
}

FunctionAST* ErrorF(const char* str)
{
	MyError(str);
	return 0;
}



//real    返回一个实数类型的节点
static ExprAST* ParseRealExpr()
{
	ExprAST* result = new  NumberRealExprAST(realNum);
	getNextToken();
	return result;
}
//int   返回一个整型节点
static ExprAST* ParseIntExpr()
{
	ExprAST* result = new NumberIntExprAST(intNum);
	getNextToken();
	return result;
}

//处理变量声明
static ExprAST* ParseVariable() {
	getNextToken();//跳过val关键字
	if (CurTok != tok_identifier) {
		return MyError("the identifier after val is wrong");
	}
	string variab = variableVal;
	getNextToken();
	if (CurTok != ':')
		return MyError("the identifier must have a type");
	getNextToken();
	//还有字符串类型
	if (CurTok != tok_real && CurTok != tok_int) {
		return MyError("the identifier type is wrong");
	}
	string typet = typeval;
	getNextToken();
	//经测试，似乎不能不赋值
	if (CurTok != '=')
		return MyError("error : identifier has no val assignment");
	getNextToken();
	//似乎关键字和真正的立即数重合了
	if (CurTok == tok_real_num) {
		if (typet.compare("real") != 0)
			return MyError("type not matching");
		else {
			getNextToken();
			return new VariableExprAST<double>(variableVal, typet, realNum);
		}
	}
	else if (CurTok == tok_int_num) {
		if (typet.compare("int") != 0)
			return MyError("type not matching");
		else {
			getNextToken();
			return new  VariableExprAST<int>(variableVal, typet, intNum);
		}
	}
}

//处理变量引用和函数调用
//sml中的函数如果不带括号，多个参数的情况不能带逗号，以空格分隔
//暂时只处理带括号和逗号的情况
static ExprAST* ParseIdentifierExpr() {
	string IDname = IdentifierStr;//已获取标识符
	getNextToken();
	//只是简单的变量
	//可以有变量类型说明可以没有
	bool hastype = false;
	if (CurTok != '(') {
		//查看是否有类型说明符
		if (CurTok == ':') {
			hastype = true;
			getNextToken();
			string type = typeval;
			if (type.compare("int") != 0 && type.compare("real") != 0) {
				return MyError("no such type");
			}
			if ("int" == type) {
				return new VariableExprAST<int>(IDname, type);
			}
			else {
				return new VariableExprAST<double>(IDname, type);
			}
		}
		//如果没有类型说明符，如何推导?
		//函数上方
		return new VariableExprAST<int>(IDname);
	}
	//函数调用
	if (!hastype)   getNextToken();
	vector<ExprAST*>Args;
	while (CurTok != ')') {//里面可能有类型说明
		ExprAST* arg = ParseExpression();
		if (!arg)return 0;
		Args.push_back(arg);
		if (CurTok == ')')break;
		if (CurTok != ',')
			return MyError("Excepted ')' or ',' in arguments");
		getNextToken();
	}
	getNextToken();//eat ')'
	//返回的主要是函数名和参数名
	return new CallExprAST(IDname, Args);
}

//主表达式
static ExprAST* ParsePrimary() {
	switch (CurTok) {
	case tok_identifier:return ParseIdentifierExpr();
	case tok_real_num:      return ParseRealExpr();//立即数操作
	case tok_int_num:       return ParseIntExpr();//立即数
	default:            return MyError("unknown token when ecpecting an expression");
	}
}
//解析有序对列表
//进行优先级解析
static ExprAST* ParseBinOpRHS(int ExprPrec, ExprAST* lhs) {
	while (1) {
		//获取下个操作符优先级
		int TokPrec = getPrecedence();
		if (TokPrec < ExprPrec)
			return lhs;
		int BinOp = CurTok;
		getNextToken();
		ExprAST* rhs = ParsePrimary();//parse后可能就是';'
		if (!rhs)return 0;
		int nextProcedence = getPrecedence();
		//如果出现有乘除的，直接将有乘除的部分合到一起作为右节点
		if (TokPrec < nextProcedence) {
			rhs = ParseBinOpRHS(TokPrec + 1, rhs);
			if (!rhs)   return 0;
		}
		lhs = new BinaryExprAST(BinOp, lhs, rhs);
	}
}
//表达式的解析
static ExprAST* ParseExpression() {
	ExprAST* lhs = ParsePrimary();//目前返回的是int立即数或real立即数或标识符
	if (!lhs) return 0;
	return ParseBinOpRHS(0, lhs);
}

//暂时不考虑注释
//括号()的解析   注意其中的递归
//这里确定括号内部是表达式
static ExprAST* ParseParenExpr() {
	getNextToken();//忽略'('
	ExprAST* re = ParseExpression();
	if (!re) return 0;
	//解析表达式后一定是')'
	if (CurTok != ')')
		return MyError("expected ')'");
	getNextToken();
	return re;
}




//解析函数原型
//返回的是函数名和函数参数的结点
static PrototypeAST* ParsePrototype() {
	if (CurTok != tok_identifier)
		return ErrorP("expected function name in prototype");
	string fnName = IdentifierStr;
	getNextToken();
	if (CurTok != '(') {
		return ErrorP("expected '(' in prototype");
	}
	vector<string> Args;
	getNextToken();
	while (CurTok != ')') {
		if (CurTok != tok_identifier)
			return ErrorP("error,no identifier!");
		string IDname = IdentifierStr;
		getNextToken();
		if (CurTok == ':') {
			getNextToken();
			if (CurTok != tok_int && CurTok != tok_real)
				return ErrorP("error,expected type!");
			string type = typeval;
			if (type.compare("int") != 0 && type.compare("real") != 0) {
				return ErrorP("error,no such type");
			}
			getNextToken();
		}
		string arg;
		//这样的话如果参数有两种就会出错，vector中只会接受一种变量
		arg = IDname;
		Args.push_back(arg);
		if (CurTok != ',' && CurTok != ')') {
			return ErrorP("expected ')' in prototype");
		}
		if (CurTok == ')')break;
		getNextToken();//eat ,
	}
	if (CurTok != ')') {
		return ErrorP("expected ')' in prototype");
	}
	getNextToken();
	return new PrototypeAST(IdentifierStr, Args);
}

//函数定义
static FunctionAST* ParseDefinition() {
	getNextToken();//eat fun
	PrototypeAST* proto = ParsePrototype();
	if (!proto) return 0;
	//解析完函数前的全部的
	if (CurTok != '=') {//由于ParsePrototype最后已经跳过了')'到了下一个，如果是函数定义必须为=
		return ErrorF("expected '=' in definition");
	}
	getNextToken();//eat '='
	if (ExprAST * E = ParseExpression())
		return new FunctionAST(proto, E);
	return 0;
}

//处理函数定义
static void HandleDefition() {
	if (ParseDefinition()) {
		cout << "Parsed a function definition\n";
	}
	else {
		getNextToken();
	}
}

//处理变量定义。
static void HandleVarDefition() {
	if (ParseVariable()) {
		cout << "Parsed a variable definition\n";
	}
	else {
		getNextToken();
	}
}
//处理直接写一个表达式的情况。
static void HandleTopLevelExpression() {
	if (ParseExpression()) {
		cout << "Parsed a top-level expression\n";
	}
	else {
		getNextToken();
	}
}

//用于直接输入表达式求值
static FunctionAST* ParseTopLevelExpr() {
	if (ExprAST * E = ParseExpression()) {
		// Make an anonymous proto.
		PrototypeAST* Proto = new PrototypeAST(" ", vector<string>());
		return new FunctionAST(Proto, E);
	}
	return 0;
}




//===----------------------------------------------------------------------===//
// Code Generation
//===----------------------------------------------------------------------===//

Value* MyErrorV(const char* Str)
{
	MyError(Str);
	return 0;
}


//NumberRealExprAST
Value* NumberRealExprAST::Codegen() {
	return ConstantFP::get(TheContext, APFloat(num));
}

//NumberIntExprAST

Value* NumberIntExprAST::Codegen() {
	return ConstantInt::get(Type::getInt32Ty(TheContext), num);
}

/*
//ParameterExprAST
Value* ParameterExprAST::Codegen() {
	return;
}
*/


//BinaryExprAST
Value* BinaryExprAST::Codegen() {
	Value* L = LHS->Codegen();
	Value* R = RHS->Codegen();
	if (!L || !R)
		return nullptr;

	switch (Op) {
	case '+':
		return Builder.CreateFAdd(L, R, "addtmp");
	case '-':
		return Builder.CreateFSub(L, R, "subtmp");
	case '*':
		return Builder.CreateFMul(L, R, "multmp");
	case '<':
		L = Builder.CreateFCmpULT(L, R, "cmptmp");
		// Convert bool 0/1 to double 0.0 or 1.0
		return Builder.CreateUIToFP(L, Type::getDoubleTy(TheContext), "booltmp");
	default:
		return MyErrorV("invalid binary operator");
	}
}
//CallExprAST
Value* CallExprAST::Codegen() {
	// Look up the name in the global module table.
	Function* CalleeF = TheModule->getFunction(Callee);
	if (!CalleeF)
		return MyErrorV("Unknown function referenced");

	// If argument mismatch error.
	if (CalleeF->arg_size() != Args.size())
		return MyErrorV("Incorrect # arguments passed");

	std::vector<Value*> ArgsV;
	for (unsigned i = 0, e = Args.size(); i != e; ++i) {
		ArgsV.push_back(Args[i]->Codegen());
		if (!ArgsV.back())
			return nullptr;
	}

	return Builder.CreateCall(CalleeF, ArgsV, "calltmp");
}

//PrototypeAST
Function* PrototypeAST::Codegen() {
	// Make the function type:  double(double,double) etc.
	std::vector<Type*> Doubles(Args.size(), Type::getDoubleTy(TheContext));
	FunctionType* FT =
		FunctionType::get(Type::getDoubleTy(TheContext), Doubles, false);

	Function* F =
		Function::Create(FT, Function::ExternalLinkage, name, TheModule.get());

	// Set names for all arguments.
	unsigned Idx = 0;
	for (auto& Arg : F->args())
		Arg.setName(Args[Idx++]);
	return F;
}

//FunctionAST
Function* FunctionAST::Codegen() {
	// First, check for an existing function from a previous 'extern' declaration.
	Function* TheFunction = TheModule->getFunction(proto->getName());

	if (!TheFunction)
		TheFunction = proto->Codegen();

	if (!TheFunction)
		return nullptr;

	// Create a new basic block to start insertion into.
	BasicBlock* BB = BasicBlock::Create(TheContext, "entry", TheFunction);
	Builder.SetInsertPoint(BB);

	// Record the function arguments in the NamedValues map.
	NamedValues.clear();
	for (auto& Arg : TheFunction->args())
		NamedValues[Arg.getName()] = &Arg;

	if (Value * RetVal = body->Codegen()) {
		// Finish off the function.
		Builder.CreateRet(RetVal);

		// Validate the generated code, checking for consistency.
		verifyFunction(*TheFunction);

		return TheFunction;
	}

	// Error reading body, remove function.
	TheFunction->eraseFromParent();
	return nullptr;
}


static void mainLoop() {
	while (1) {
		cout << "ready>" << endl;
		switch (CurTok) {
		case tok_eof:	return;
		case ';':		getNextToken(); break;
		case tok_fun:  HandleDefition(); break;//处理函数定义
		case tok_val:  HandleVarDefition(); break;//处理变量定义
		default:			HandleTopLevelExpression();
		}
	}
}


