1. ##### 值和函数的声明:
###### 声明值：关键字val，eg:

		val times=60;
		> val times= 60 : int;
		val pi=3.14;
		> val pi= 3.14 : real;

###### 关于it的，在命令行会将上个输入的表达式的值存入it。

###### 函数声明
	
	fun area (r) = pi*r*r;
	//以关键字fun开始，函数名为area，参数为r，函数体（body）为pi*r*r;
	>val area= fn : real -> real;
	//无论是定义还是调用函数参数的()都是可选的

###### 注释
	以(*开始，以*)结束，可以跨越数行。
	变量可以重新声明，但是不会影响之前的函数中用到的变量的值。即函数只会调用其之前的环境。静态绑定。

##### 标识符
	必须以字母开始，后面可以跟随任意的字母，数字，下划线，或者单引号。大小写区别。允许单引号是方便数学。一定要避免使用ML的关键字。
	也可以使用符号名字：
	! % & $ # + - * / :  < = > ? @ \ ~ ' ^ | 可以无限长。
	但是下面一些是保留的。
	: | = => -> # :>
	字母名字可以出现的地方，符号名字也可以出现。
##### 整数和实数的区分。
	整数。 一串数字，可能以一个~开始： 0 ~23 01234 ~85312321
	运算为+ - * div mod 遵循优先级和平时相同。
	实数。 一个实数常量包含一个小数点或者E记法，或者两者都有。
	0.01 2.41832 ~1.2E12 7E~5
	En为乘以10的n次方。
	area a + b 等价于(area a) + b
	类型约束：
	如下：ML不能确定是给整数用的还是实数用的，因此会拒绝接受声明。eg：
		
	fun square x = x*x;//error ,unable to resolve overloading for *
	fun square (x: real) = x*x; //real->real
	fun square x :real =x*x;//先指定函数的结果的类型为real
	fun square x =x*x :real;//指定函数体的类型。
##### 字符串和字符
	字符串类型：
	"dassdada";
	>"dassdada" : string
	^可以将两个字符串首尾相接。
	"Fair "^"Ophelia"
	>"Fair Ophelia" :string
	size(it);
	>12 : int
	fun title(name) = “"the Duke of"^name;
	>val title = fn: string -> string
	title "York"
	\n	换行符
	\t 制表符
	\"  双引号
	\\  反斜线
	\之后紧跟一个换行或者其他空白字符，然后再跟一个\则不插入任何字符。不过在换行之后续写同一个字符串。
	字符类型：
	形如#s,s是只有一个字符的字符串常量。
	#"a" #" "  #"\n"
##### 真值和条件表达式
	条件表达式：if E the E1 else E2;
	fun sign(n) = 
					if n > 0 then 1
					else if n =0 then 0
					else (*n<0*) then ~1;//注意负号为~
	>val sign = fn  : int -> int
	fun isLower c = #"a" <= c andalso c <=#"z";
	>val isLower = fn : char -> bool
#####  序偶，元祖和记录
	序偶（二元组）
	(2.5,~1.2);
	> (2.5,~1.2): real*real
	fun average(x,y) =(x+y)/2.0;
	>val average=fn:(real*real) ->real
	((2.0,3.5),zerovec);
	>((2.0,3.5),(0.0,0.0)):(real*real)*(real*real)
	fun addvec((x1,y1),(x2,y2)):vec = (x1+x2,y1+y2);
	> val addvec = fn :(real*real)*(real*real)->vec
	记录
	具有标签的元组//类似map
	val henryVI = 
	{ name = "Henry VI",
		  born = 1421,
		  crowned =  1422,
		  died = 1471,
		  };
	>val henryVI=
	{ born = 1421,
	  crowned = 1422,
	  died = 1471,
	  name = "Henry VI"
	  }
	:{born :int,
	 crowned :int,
	 died : int,
	 name:string}

	 提取元组部分
	 #born henryVI
	 >born : int
	 n元组：
	 {1=x1,2=x2,......,n=xn}
	 #2 {"a","b",3,false}
	 >"b" : string
	声明记录类型
	type king={name :string,
					born : int,
					died : int};
	>type king
	fun lifetime(k:king) = #died k - #born k;
	> val lifetime = fn:king->int
	fun lifetime ({born,died,...}:king) = died-born;
	> val lifetime = fn:king->int
##### 中缀操作符
	infix指令可以声明一个从0到9的优先级。默认的优先级是0.
	infix 6 plus;
	fun ( a plus b ) = "(" ^ a ^ "+" ^ b ^ ")";
	>val plus = fn :string*string ->string
	"1" plus "2" plus "3";
	> "((1+2)+3)":string
	两个符号名字必须用空格或者其他字符隔开
	1 +~3//error，unknown name +~
	1+ ~3//right
	infix ++;//可以自己写定义
	fun ((x1,y1) ++ (x2,y2)):vec = (x1+x2,y1+y2);
	> val ++ = fn :(real*real) *(real*real)->vec
	op关键字
	op++ ((2.5),(0.1,2.5));
	 (2.6,2.5):real*real
##### 表达式的求值
	传值调用
	E1 andalso E2是下式的简写
	if E1 the E2 else false
##### 局部声明
	fun fraction (n,d)=
		let val com= gcd(n,d)
		in (n div com,d div com)  end;
	>val fraction = fn:int *int ->int *int
	let表达式 let D in E end
	D通常是复合声明，D1;D2;D3;...;Dn  分号可选
	local 可以隐藏声明
	local D1 in D2 end
	D1只在D2内可见。
	
	 
