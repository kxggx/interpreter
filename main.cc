#include"ast_oper.h"

int main(){
	BinOpPrecedence['<'] = 10;
	BinOpPrecedence['>'] = 10;
	BinOpPrecedence['+'] = 20;
	BinOpPrecedence['-'] = 20;
	BinOpPrecedence['*'] = 40;  // highest.
 	BinOpPrecedence['/']	= 40;
 	cout<<"ready>\n";
  	getNextToken();
  	mainLoop();
  	return 0;
}
