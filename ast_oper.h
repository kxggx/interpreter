#include"ast.h" 

//�������ñ���׺��������ȼ����ߣ���˿��Բ������š�

//�ٶ���������ָ������
//�ٶ��������������
// + - * 
// int ����Ϊdiv realΪ/ ȡģ mod
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
//��ȡ�����������ȼ�  //���ǲ���������div mod 
//isascii�ж������Ƿ���1-127֮��
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



//real    ����һ��ʵ�����͵Ľڵ�
static ExprAST* ParseRealExpr()
{
	ExprAST* result = new  NumberRealExprAST(realNum);
	getNextToken();
	return result;
}
//int   ����һ�����ͽڵ�
static ExprAST* ParseIntExpr()
{
	ExprAST* result = new NumberIntExprAST(intNum);
	getNextToken();
	return result;
}

//�����������
static ExprAST* ParseVariable() {
	getNextToken();//����val�ؼ���
	if (CurTok != tok_identifier) {
		return MyError("the identifier after val is wrong");
	}
	string variab = variableVal;
	getNextToken();
	if (CurTok != ':')
		return MyError("the identifier must have a type");
	getNextToken();
	//�����ַ�������
	if (CurTok != tok_real && CurTok != tok_int) {
		return MyError("the identifier type is wrong");
	}
	string typet = typeval;
	getNextToken();
	//�����ԣ��ƺ����ܲ���ֵ
	if (CurTok != '=')
		return MyError("error : identifier has no val assignment");
	getNextToken();
	//�ƺ��ؼ��ֺ��������������غ���
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

//����������úͺ�������
//sml�еĺ�������������ţ����������������ܴ����ţ��Կո�ָ�
//��ʱֻ��������źͶ��ŵ����
static ExprAST* ParseIdentifierExpr() {
	string IDname = IdentifierStr;//�ѻ�ȡ��ʶ��
	getNextToken();
	//ֻ�Ǽ򵥵ı���
	//�����б�������˵������û��
	bool hastype = false;
	if (CurTok != '(') {
		//�鿴�Ƿ�������˵����
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
		//���û������˵����������Ƶ�?
		//�����Ϸ�
		return new VariableExprAST<int>(IDname);
	}
	//��������
	if (!hastype)   getNextToken();
	vector<ExprAST*>Args;
	while (CurTok != ')') {//�������������˵��
		ExprAST* arg = ParseExpression();
		if (!arg)return 0;
		Args.push_back(arg);
		if (CurTok == ')')break;
		if (CurTok != ',')
			return MyError("Excepted ')' or ',' in arguments");
		getNextToken();
	}
	getNextToken();//eat ')'
	//���ص���Ҫ�Ǻ������Ͳ�����
	return new CallExprAST(IDname, Args);
}

//�����ʽ
static ExprAST* ParsePrimary() {
	switch (CurTok) {
	case tok_identifier:return ParseIdentifierExpr();
	case tok_real_num:      return ParseRealExpr();//����������
	case tok_int_num:       return ParseIntExpr();//������
	default:            return MyError("unknown token when ecpecting an expression");
	}
}
//����������б�
//�������ȼ�����
static ExprAST* ParseBinOpRHS(int ExprPrec, ExprAST* lhs) {
	while (1) {
		//��ȡ�¸����������ȼ�
		int TokPrec = getPrecedence();
		if (TokPrec < ExprPrec)
			return lhs;
		int BinOp = CurTok;
		getNextToken();
		ExprAST* rhs = ParsePrimary();//parse����ܾ���';'
		if (!rhs)return 0;
		int nextProcedence = getPrecedence();
		//��������г˳��ģ�ֱ�ӽ��г˳��Ĳ��ֺϵ�һ����Ϊ�ҽڵ�
		if (TokPrec < nextProcedence) {
			rhs = ParseBinOpRHS(TokPrec + 1, rhs);
			if (!rhs)   return 0;
		}
		lhs = new BinaryExprAST(BinOp, lhs, rhs);
	}
}
//���ʽ�Ľ���
static ExprAST* ParseExpression() {
	ExprAST* lhs = ParsePrimary();//Ŀǰ���ص���int��������real���������ʶ��
	if (!lhs) return 0;
	return ParseBinOpRHS(0, lhs);
}

//��ʱ������ע��
//����()�Ľ���   ע�����еĵݹ�
//����ȷ�������ڲ��Ǳ��ʽ
static ExprAST* ParseParenExpr() {
	getNextToken();//����'('
	ExprAST* re = ParseExpression();
	if (!re) return 0;
	//�������ʽ��һ����')'
	if (CurTok != ')')
		return MyError("expected ')'");
	getNextToken();
	return re;
}




//��������ԭ��
//���ص��Ǻ������ͺ��������Ľ��
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
		//�����Ļ�������������־ͻ����vector��ֻ�����һ�ֱ���
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

//��������
static FunctionAST* ParseDefinition() {
	getNextToken();//eat fun
	PrototypeAST* proto = ParsePrototype();
	if (!proto) return 0;
	//�����꺯��ǰ��ȫ����
	if (CurTok != '=') {//����ParsePrototype����Ѿ�������')'������һ��������Ǻ����������Ϊ=
		return ErrorF("expected '=' in definition");
	}
	getNextToken();//eat '='
	if (ExprAST * E = ParseExpression())
		return new FunctionAST(proto, E);
	return 0;
}

//����������
static void HandleDefition() {
	if (ParseDefinition()) {
		cout << "Parsed a function definition\n";
	}
	else {
		getNextToken();
	}
}

//����������塣
static void HandleVarDefition() {
	if (ParseVariable()) {
		cout << "Parsed a variable definition\n";
	}
	else {
		getNextToken();
	}
}
//����ֱ��дһ�����ʽ�������
static void HandleTopLevelExpression() {
	if (ParseExpression()) {
		cout << "Parsed a top-level expression\n";
	}
	else {
		getNextToken();
	}
}

//����ֱ��������ʽ��ֵ
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
		case tok_fun:  HandleDefition(); break;//����������
		case tok_val:  HandleVarDefition(); break;//�����������
		default:			HandleTopLevelExpression();
		}
	}
}


