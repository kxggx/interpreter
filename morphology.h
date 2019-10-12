#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>

//===----------------------------------------------------------------------===//
// Lexer
//===----------------------------------------------------------------------===//

// The lexer returns tokens [0-255] if it is an unknown character, otherwise one
// of these for known things.
enum Token
{
	tok_eof = -1,

	tok_abstype = -2,
	tok_and = -3,
	tok_andalso = -4,
	tok_as = -5,
	tok_case = -6,
	tok_datatype = -7,
	tok_do = -8,
	tok_else = -9,
	tok_end = -10,
	tok_eqtype = -11,
	tok_exception = -12,
	tok_fn = -13,
	tok_fun = -14,
	tok_functor = -15,
	tok_handle = -16,
	tok_if = -17,
	tok_in = -18,
	tok_include = -19,
	tok_infix = -20,
	tok_infixr = -21,
	tok_let = -22,
	tok_local = -23,

	tok_nonfix = -24,
	tok_of = -25,
	tok_op = -26,
	tok_open = -27,
	tok_orelse = -28,
	tok_raise = -29,
	tok_rec = -30,
	tok_sharing = -31,
	tok_sig = -32,
	tok_signature = -33,
	tok_struct = -34,
	tok_structure = -35,
	tok_then = -36,
	tok_type = -37,
	tok_val = -38,
	tok_where = -39,
	tok_while = -40,
	tok_with = -41,
	tok_withtype = -42,
	tok_identifier = -43,
	tok_number = -44,

	tok_int = -45,
	tok_real = -46,
	tok_not = -47,

};

static std::string IdentifierStr; // Filled in if tok_identifier
static double NumVal;			  // Filled in if tok_number

/// gettok - Return the next token from standard input.
static int gettok()
{
	static int LastChar = ' ';

	// Skip any whitespace.
	while (isspace(LastChar))
		LastChar = getchar();

	if (isalpha(LastChar))
	{ // identifier: [a-zA-Z][a-zA-Z0-9]*
		IdentifierStr = LastChar;
		while (isalnum((LastChar = getchar())))
			IdentifierStr += LastChar;
		if (IdentifierStr == "abstype")
			return tok_abstype;
		if (IdentifierStr == "and")
			return tok_and;
		if (IdentifierStr == "andalso")
			return tok_andalso;
		if (IdentifierStr == "as")
			return tok_andalso;
		if (IdentifierStr == "case")
			return tok_case;
		if (IdentifierStr == "datatype")
			return tok_datatype;
		if (IdentifierStr == "do")
			return tok_do;
		if (IdentifierStr == "else")
			return tok_else;
		if (IdentifierStr == "end")
			return tok_end;
		if (IdentifierStr == "eqtype")
			return tok_eqtype;
		if (IdentifierStr == "exception")
			return tok_exception;
		if (IdentifierStr == "fn")
			return tok_fn;
		if (IdentifierStr == "fun")
			return tok_fun;
		if (IdentifierStr == "functor")
			return tok_functor;
		if (IdentifierStr == "handle")
			return tok_handle;
		if (IdentifierStr == "if")
			return tok_if;
		if (IdentifierStr == "in")
			return tok_in;
		if (IdentifierStr == "include")
			return tok_include;
		if (IdentifierStr == "infix")
			return tok_infix;
		if (IdentifierStr == "infixr")
			return tok_infixr;
		if (IdentifierStr == "let")
			return tok_let;
		if (IdentifierStr == "local")
			return tok_local;
		if (IdentifierStr == "nonfix")
			return tok_nonfix;
		if (IdentifierStr == "of")
			return tok_of;
		if (IdentifierStr == "op")
			return tok_op;
		if (IdentifierStr == "open")
			return tok_open;
		if (IdentifierStr == "orelse")
			return tok_orelse;
		if (IdentifierStr == "raise")
			return tok_raise;
		if (IdentifierStr == "rec")
			return tok_rec;
		if (IdentifierStr == "sharing")
			return tok_sharing;
		if (IdentifierStr == "sig")
			return tok_sig;
		if (IdentifierStr == "signature")
			return tok_signature;
		if (IdentifierStr == "struct")
			return tok_struct;
		if (IdentifierStr == "structure")
			return tok_structure;
		if (IdentifierStr == "then")
			return tok_then;
		if (IdentifierStr == "type")
			return tok_type;
		if (IdentifierStr == "val")
			return tok_val;
		if (IdentifierStr == "where")
			return tok_where;
		if (IdentifierStr == "while")
			return tok_while;
		if (IdentifierStr == "with")
			return tok_with;
		if (IdentifierStr == "withtype")
			return tok_withtype;
		if (IdentifierStr == "identifier")
			return tok_identifier;
		if (IdentifierStr == "number")
			return tok_number;
		return tok_identifier;
	}

	if (isdigit(LastChar) || LastChar == '.' || LastChar == '~' || LastChar == 'E')
	{ // Number: [0-9.]+
		printf("location: %d\n");
		printf("accept a number!\n");

		std::string NumStr;
		do
		{
			NumStr += LastChar;
			LastChar = getchar();
		} while (isdigit(LastChar) || LastChar == '.' || LastChar == '~' || LastChar == 'E');

		NumVal = strtod(NumStr.c_str(), nullptr);
		printf("number is %f\n", NumVal);
		return tok_number;
	}

	if (LastChar == '#')
	{
		// Comment until end of line.
		do
			LastChar = getchar();
		while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

		if (LastChar != EOF)
			return gettok();
	}

	// Check for end of file.  Don't eat the EOF.
	if (LastChar == EOF)
		return tok_eof;

	// Otherwise, just return the character as its ascii value.
	int ThisChar = LastChar;
	LastChar = getchar();
	return ThisChar;
}

//===----------------------------------------------------------------------===//
// Abstract Syntax Tree (aka Parse Tree)
//===----------------------------------------------------------------------===//

namespace
{

/// ExprAST - Base class for all expression nodes.
class ExprAST
{
public:
	virtual ~ExprAST() = default;
};

/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST
{
	double Val;

public:
	NumberExprAST(double Val) : Val(Val) {}
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST
{
	std::string Name;

public:
	VariableExprAST(const std::string &Name) : Name(Name) {}
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST
{
	char Op;
	std::unique_ptr<ExprAST> LHS, RHS;

public:
	BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS,
				  std::unique_ptr<ExprAST> RHS)
		: Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST
{
	std::string Callee;
	std::vector<std::unique_ptr<ExprAST>> Args;

public:
	CallExprAST(const std::string &Callee,
				std::vector<std::unique_ptr<ExprAST>> Args)
		: Callee(Callee), Args(std::move(Args)) {}
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class PrototypeAST
{
	std::string Name;
	std::vector<std::string> Args;

public:
	PrototypeAST(const std::string &Name, std::vector<std::string> Args)
		: Name(Name), Args(std::move(Args)) {}

	const std::string &getName() const { return Name; }
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST
{
	std::unique_ptr<PrototypeAST> Proto;
	std::unique_ptr<ExprAST> Body;

public:
	FunctionAST(std::unique_ptr<PrototypeAST> Proto,
				std::unique_ptr<ExprAST> Body)
		: Proto(std::move(Proto)), Body(std::move(Body)) {}
};

} // end anonymous namespace

//===----------------------------------------------------------------------===//
// Parser
//===----------------------------------------------------------------------===//

/// CurTok/getNextToken - Provide a simple token buffer.  CurTok is the current
/// token the parser is looking at.  getNextToken reads another token from the
/// lexer and updates CurTok with its results.
static int CurTok;
static int getNextToken() { return CurTok = gettok(); }

/// BinopPrecedence - This holds the precedence for each binary operator that is
/// defined.
static std::map<char, int> BinopPrecedence;

/// GetTokPrecedence - Get the precedence of the pending binary operator token.
static int GetTokPrecedence()
{
	if (!isascii(CurTok))
		return -1;

	// Make sure it's a declared binop.
	int TokPrec = BinopPrecedence[CurTok];
	if (TokPrec <= 0)
		return -1;
	return TokPrec;
}

/// LogError* - These are little helper functions for error handling.
std::unique_ptr<ExprAST> LogError(const char *Str)
{
	fprintf(stderr, "Error: %s\n", Str);
	return nullptr;
}
std::unique_ptr<PrototypeAST> LogErrorP(const char *Str)
{
	LogError(Str);
	return nullptr;
}

static std::unique_ptr<ExprAST> ParseExpression();

/// numberexpr ::= number
static std::unique_ptr<ExprAST> ParseNumberExpr()
{
	auto Result = llvm::make_unique<NumberExprAST>(NumVal);
	getNextToken(); // consume the number
	return std::move(Result);
}

/// parenexpr ::= '(' expression ')'
static std::unique_ptr<ExprAST> ParseParenExpr()
{
	getNextToken(); // eat (.
	auto V = ParseExpression();
	if (!V)
		return nullptr;

	if (CurTok != ')')
		return LogError("expected ')'");
	getNextToken(); // eat ).
	return V;
}

/// identifierexpr
///   ::= identifier
///   ::= identifier '(' expression* ')'
static std::unique_ptr<ExprAST> ParseIdentifierExpr()
{
	std::string IdName = IdentifierStr;

	getNextToken(); // eat identifier.

	if (CurTok != '(') // Simple variable ref.
		return llvm::make_unique<VariableExprAST>(IdName);

	// Call.
	getNextToken(); // eat (
	std::vector<std::unique_ptr<ExprAST>> Args;
	if (CurTok != ')')
	{
		while (true)
		{
			if (auto Arg = ParseExpression())
				Args.push_back(std::move(Arg));
			else
				return nullptr;

			if (CurTok == ')')
				break;

			if (CurTok != ',')
				return LogError("Expected ')' or ',' in argument list");
			getNextToken();
		}
	}

	// Eat the ')'.
	getNextToken();

	return llvm::make_unique<CallExprAST>(IdName, std::move(Args));
}

/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr
static std::unique_ptr<ExprAST> ParsePrimary()
{
	switch (CurTok)
	{
	default:
		return LogError("unknown token when expecting an expression");
	case tok_identifier:
		return ParseIdentifierExpr();
	case tok_number:
		return ParseNumberExpr();
	case '(':
		return ParseParenExpr();
	}
}

/// binoprhs
///   ::= ('+' primary)*
static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
											  std::unique_ptr<ExprAST> LHS)
{
	// If this is a binop, find its precedence.
	while (true)
	{
		int TokPrec = GetTokPrecedence();

		// If this is a binop that binds at least as tightly as the current binop,
		// consume it, otherwise we are done.
		if (TokPrec < ExprPrec)
			return LHS;

		// Okay, we know this is a binop.
		int BinOp = CurTok;
		getNextToken(); // eat binop

		// Parse the primary expression after the binary operator.
		auto RHS = ParsePrimary();
		if (!RHS)
			return nullptr;

		// If BinOp binds less tightly with RHS than the operator after RHS, let
		// the pending operator take RHS as its LHS.
		int NextPrec = GetTokPrecedence();
		if (TokPrec < NextPrec)
		{
			RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
			if (!RHS)
				return nullptr;
		}

		// Merge LHS/RHS.
		LHS = llvm::make_unique<BinaryExprAST>(BinOp, std::move(LHS),
											   std::move(RHS));
	}
}

/// expression
///   ::= primary binoprhs
///
static std::unique_ptr<ExprAST> ParseExpression()
{
	auto LHS = ParsePrimary();
	if (!LHS)
		return nullptr;

	return ParseBinOpRHS(0, std::move(LHS));
}

/// prototype
///   ::= id '(' id* ')'
static std::unique_ptr<PrototypeAST> ParsePrototype()
{
	if (CurTok != tok_identifier)
		return LogErrorP("Expected function name in prototype");

	std::string FnName = IdentifierStr;
	getNextToken();

	if (CurTok != '(')
		return LogErrorP("Expected '(' in prototype");

	std::vector<std::string> ArgNames;
	while (getNextToken() == tok_identifier)
		ArgNames.push_back(IdentifierStr);
	if (CurTok != ')')
		return LogErrorP("Expected ')' in prototype");

	// success.
	getNextToken(); // eat ')'.

	return llvm::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
}

/// definition ::= 'def' prototype expression
static std::unique_ptr<FunctionAST> ParseDefinition()
{
	getNextToken(); // eat def.
	auto Proto = ParsePrototype();
	if (!Proto)
		return nullptr;

	if (auto E = ParseExpression())
		return llvm::make_unique<FunctionAST>(std::move(Proto), std::move(E));
	return nullptr;
}

/// toplevelexpr ::= expression
static std::unique_ptr<FunctionAST> ParseTopLevelExpr()
{
	if (auto E = ParseExpression())
	{
		// Make an anonymous proto.
		auto Proto = llvm::make_unique<PrototypeAST>("__anon_expr",
													 std::vector<std::string>());
		return llvm::make_unique<FunctionAST>(std::move(Proto), std::move(E));
	}
	return nullptr;
}

/// external ::= 'extern' prototype
static std::unique_ptr<PrototypeAST> ParseExtern()
{
	getNextToken(); // eat extern.
	return ParsePrototype();
}

//===----------------------------------------------------------------------===//
// Top-Level parsing
//===----------------------------------------------------------------------===//

static void HandleDefinition()
{
	if (ParseDefinition())
	{
		fprintf(stderr, "Parsed a function definition.\n");
	}
	else
	{
		// Skip token for error recovery.
		getNextToken();
	}
}

static void HandleExtern()
{
	if (ParseExtern())
	{
		fprintf(stderr, "Parsed an extern\n");
	}
	else
	{
		// Skip token for error recovery.
		getNextToken();
	}
}

static void HandleTopLevelExpression()
{
	// Evaluate a top-level expression into an anonymous function.
	if (ParseTopLevelExpr())
	{
		fprintf(stderr, "Parsed a top-level expr\n");
	}
	else
	{
		// Skip token for error recovery.
		getNextToken();
	}
}

static void HandleKeyWord(std::string key)
{
	cout << "location: " << mycount << endl;
	cout << "get a " << key << endl;
	getNextToken();
}
/// top ::= definition | external | expression | ';'
static void MainLoop()
{
	while (true)
	{
		fprintf(stderr, "ready> ");
		switch (CurTok)
		{
		case tok_eof:
			return;
		case ';': // ignore top-level semicolons.
			getNextToken();
			break;
		case tok_abstype:
			HandleKeyWord("abstype");
			break;
		case tok_and:
			HandleKeyWord("and");
			break;
		case tok_andalso:
			HandleKeyWord("andalso");
			break;
		case tok_as:
			HandleKeyWord("as");
			break;
		case tok_case:
			HandleKeyWord("case");
			break;
		case tok_datatype:
			HandleKeyWord("datatype");
			break;
		case tok_do:
			HandleKeyWord("do");
			break;
		case tok_else:
			HandleKeyWord("else");
			break;
		case tok_end:
			HandleKeyWord("end");
			break;
		case tok_eqtype:
			HandleKeyWord("eqtype");
			break;
		case tok_exception:
			HandleKeyWord("exception");
			break;
		case tok_fn:
			HandleKeyWord("fn");
			break;
		case tok_fun:
			HandleKeyWord("fun");
			break;
		case tok_functor:
			HandleKeyWord("functor");
			break;
		case tok_handle:
			HandleKeyWord("handle");
			break;
		case tok_if:
			HandleKeyWord("if");
			break;
		case tok_in:
			HandleKeyWord("in");
			break;
		case tok_include:
			HandleKeyWord("include");
			break;
		case tok_infix:
			HandleKeyWord("infix");
			break;
		case tok_infixr:
			HandleKeyWord("infixr");
			break;
		case tok_let:
			HandleKeyWord("let");
			break;
		case tok_local:
			HandleKeyWord("local");
			break;
		case tok_nonfix:
			HandleKeyWord("nonfix");
			break;
		case tok_of:
			HandleKeyWord("of");
			break;
		case tok_op:
			HandleKeyWord("op");
			break;
		case tok_open:
			HandleKeyWord("open");
			break;
		case tok_orelse:
			HandleKeyWord("orelse");
			break;
		case tok_raise:
			HandleKeyWord("raise");
			break;
		case tok_rec:
			HandleKeyWord("rec");
			break;
		case tok_sharing:
			HandleKeyWord("sharing");
			break;
		case tok_sig:
			HandleKeyWord("sig");
			break;
		case tok_signature:
			HandleKeyWord("signature");
			break;
		case tok_struct:
			HandleKeyWord("struct");
			break;
		case tok_structure:
			HandleKeyWord("structure");
			break;
		case tok_then:
			HandleKeyWord("then");
			break;
		case tok_type:
			HandleKeyWord("type");
			break;
		case tok_val:
			HandleKeyWord("val");
			break;
		case tok_where:
			HandleKeyWord("where");
			break;
		case tok_while:
			HandleKeyWord("while");
			break;
		case tok_with:
			HandleKeyWord("with");
			break;
		case tok_withtype:
			HandleKeyWord("withtype");
			break;
		case tok_identifier:
			HandleKeyWord("identifier");
			break;
		case tok_number:
			HandleKeyWord("number");
			break;
		case tok_int:
			HandleKeyWord("int");
			break;
		case tok_real:
			HandleKeyWord("real");
			break;
		case tok_not:
			HandleKeyWord("not");
			break;

		default:
			HandleTopLevelExpression();
			break;
		}
	}
}

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//

int main()
{
	// Install standard binary operators.
	// 1 is lowest precedence.
	BinopPrecedence['<'] = 10;
	BinopPrecedence['+'] = 20;
	BinopPrecedence['-'] = 20;
	BinopPrecedence['*'] = 40; // highest.

	// Prime the first token.
	fprintf(stderr, "ready> ");
	getNextToken();

	// Run the main "interpreter loop" now.
	MainLoop();

	return 0;
}
