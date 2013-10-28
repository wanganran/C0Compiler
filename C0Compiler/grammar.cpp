#include "compiler.h"
#include "grammar.h"
#include "structures.h"
#include "table.h"
static int currentLayer=0;
//start 2nd-class(if error, return null and words won't change) nodes.
static Node* getNumber(Word*& words)
{
	if(inarr(words[0].type,2,PLUS,MINUS))
	{
		if(words[1].type==UNSIGNED){
			words=words+2;
			return new Node(Number,2,new Node(words-2),new Node(words-1));
		}
		else{
			error(GRAMMARERR,"数字定义有误",words[0].line,words[1].col);
			return NULL;
		}
	}
	else if(words[0].type==UNSIGNED||words[0].type==ZERO)
	{
		words=words+1;
		return new Node(Number,1,new Node(words-1));
	}
	else
	{
		error(GRAMMARERR,"数字定义有误",words[0].line,words[1].col);
		return NULL;
	}
}
static Node* getOperator(Word*& words)
{
	if(inarr(words->type,6,LT,LTET,RT,RTET,ET,NET))
	{
		words=words+1;
		return new Node(Operator,1,new Node(words-1));
	}
	else
	{
		error(GRAMMARERR,"比较运算符定义有误",words[0].line,words[1].col);
		return NULL;
	}
}
static Node* getExpression(Word*& words);
static Node* getFactor(Word*& words)
{
	Word* back=words;
	Node* res=new Node(Factor,0);
	Node* tmp=NULL;
	if(words->type==LPAREN)
	{
		res->addChild(new Node(words++));
		if((tmp=getExpression(words))!=0)
			res->addChild(tmp);
		else
		{
			goto err;
		}
		if(words->type==RPAREN)
		{
			res->addChild(new Node(words++));
			return res;
		}
		else
		{
			goto err;
		}
	}
	else if(words->type==IDENTIFIER)
	{
		res->addChild(new Node(words++));
		if(words->type==LPAREN)
		{
			//it's a function
			res->addChild(new Node(words++));
			if(words->type==RPAREN)
			{
				res->addChild(new Node(words++));
				return res;
			}
			while(1)
			{
				if((tmp=getExpression(words))!=0)
					res->addChild(tmp);
				else
				{
					goto err;
				}
				if(words->type==COMMA)
					res->addChild(new Node(words++));
				else if(words->type==RPAREN){
					res->addChild(new Node(words++));
					return res;
				}
				else
				{
					goto err;
				}
			}
		}
		else if(words->type==LBRACHET)
		{
			//it's a array
			res->addChild(new Node(words++));
			if((tmp=getExpression(words))!=0)
				res->addChild(tmp);
			else
			{
				goto err;
			}
			if(words->type!=RBRACHET)
			{
				goto err;
			}
			res->addChild(new Node(words++));
			return res;
		}
		else
			return res;
	}
	else if(words->type==CHAR)
	{
		res->addChild(new Node(words++));
		return res;
	}
	else{
		Node* r=0;
		if((r=getNumber(words))!=0)
		{
			res->addChild(r);
			return res;
		}
		else
		{
			goto err;
		}
	}
err:
	error(GRAMMARERR,"因子定义有误",words->line,words->col);
	words=back;
	return NULL;
}
static Node* getTerm(Word*& words)
{
	Word* back=words;
	Node* res=new Node(Term,0);
	while(true)
	{
		Node* t=getFactor(words);
		if(t==NULL)
		{
			goto err;
		}
		res->addChild(t);
		if(!inarr(words->type,2,MULTIPLY,DIVIDE))
		{
			return res;
		}
		res->addChild(new Node(words++));
	}
err:
	error(GRAMMARERR,"项定义有误",words->line,words->col);
	words=back;
	return NULL;
}
static Node* getExpression(Word*& words)
{
	Word* back=words;
	Node* res=new Node(Expression,0);
	if(inarr(words->type,2,PLUS,MINUS))
		res->addChild(new Node(words++));
	while (1)
	{
		Node* t=getTerm(words);
		if(t==NULL)
		{
			goto err;
		}
		res->addChild(t);
		if(!inarr(words->type,2,PLUS,MINUS))
			return res;
		res->addChild(new Node(words++));
	}
err:
	error(GRAMMARERR,"表达式定义有误",words->line,words->col);
	words=back;
	return NULL;
}
static Node* getJudgement(Word*& words)
{
	Word* back=words;
	Node* res=new Node(Judgement,0);
	Node* t=getExpression(words);
	if(t==NULL)
	{
		goto err;
	}
	res->addChild(t);
	t=getOperator(words);
	if(t==NULL)
	{
		return res;
	}
	res->addChild(t);
	t=getExpression(words);
	if(t==NULL)
	{
		goto err;
	}
	res->addChild(t);
	return res;
err:
	error(GRAMMARERR,"条件定义有误",words->line,words->col);
	words=back;
	return NULL;
}

//begin top-layer(always returns a node. If error, words will jump to the nearest word that wont get that error again) nodes.
static Node* getConsts(Word*& words)
{
	Node* result=new Node(Consts,0);
	Node* t;
	while(true){
		bool ischar=false;
		if(words->type!=CONST)return result;
		Node* res=like(Const,words,5,CONST,INT,IDENTIFIER,ASSIGN,Number);
		if(!res&&(ischar=true))
			res=like(Const,words,5,CONST,CHR,IDENTIFIER,ASSIGN,CHAR);
		if(!res){
			error(GRAMMARERR,"常量定义有误",words->line,words->col);
			find(words,SEMICOLON);
			if(words->type==ENDOFFILE)return NULL;
			words++;
			if(words->type==CONST)
				continue;
			/*else if(!inarr(words->type,3,INT,CHR,VOID))
				error(GRAMMARERR,"未识别标识符",words->line,words->col);*/
			return result;
		}
		goto check;
rescan:
		if(!ischar)t=like(Const,words,3,IDENTIFIER,ASSIGN,Number);
		else t=like(Const,words,3,IDENTIFIER,ASSIGN,CHAR);
		if(!t){
			error(GRAMMARERR,"常量定义有误",words->line,words->col);
			find(words,SEMICOLON);

			if(words->type==ENDOFFILE)return NULL;
			words++;
			if(words->type==CONST)
				continue;
			/*else if(!inarr(words->type,3,INT,CHR,VOID))
				error(GRAMMARERR,"未识别标识符",words->line,words->col);
				*/
			return result;
		}
		res->Merge(t);
check:
		if(words->type==SEMICOLON){
			res->addChild(new Node(words++));
			result->addChild(res);
		}
		else if(words->type==COMMA){
			res->addChild(new Node(words++));
			goto rescan;
		}
		else
		{	
			error(GRAMMARERR,"未识别标识符",words->line,words->col);
			find(words,SEMICOLON);
			if(words->type==ENDOFFILE)return NULL;
			words++;
			if(words->type==CONST)
				continue;
			/*else if(!inarr(words->type,3,INT,CHR,VOID))
				error(GRAMMARERR,"未识别标识符",words->line,words->col);*/
			return result;
		}
	}
	return result;
}
static Node* getDeclares(Word*& words)
{
	Node* res=new Node(Declarations,0);
	
	while(true){
		Node* tmp=new Node(Declaration,0);
		bool rescaned=false;
	
		if(!inarr(words->type,2,INT,CHR))
			return res;
		tmp->addChild(new Node(words));
rescan:
		if(words[1].type!=IDENTIFIER)
		{
			error(GRAMMARERR,"变量/函数名称定义有误",words[1].line,words[1].col);
			find(words,SEMICOLON);
			if(words->type==ENDOFFILE)return NULL;
			words++;
			continue;
		}
		tmp->addChild(new Node(words+1));
		
		int t=2;
		if(words[2].type==LBRACHET)
		{
			if(words[3].type==UNSIGNED&&words[4].type==RBRACHET)
			{
				tmp->addChild(new Node(words+2));
				tmp->addChild(new Node(words+3));
				tmp->addChild(new Node(words+4));
				t=5;
			}
			else
			{
				error(GRAMMARERR,"数组变量定义有误",words[3].line,words[3].col);
				find(words,SEMICOLON);
			if(words->type==ENDOFFILE)return NULL;
				words++;
				if(!inarr(words->type,2,INT,CHR))
					return res;
				else
					continue;
			}
		}
		else if(words[2].type==LPAREN)
		{	
			if(!rescaned)
				return res;
			else{
				error(GRAMMARERR,"变量定义有误",words[1].line,words[1].col);
				find(words,SEMICOLON);
			if(words->type==ENDOFFILE)return NULL;
				words++;
				if(!inarr(words->type,2,INT,CHR))
					return res;
				else
					continue;
			}
		}
		words+=t;
		if(words->type==SEMICOLON){
			tmp->addChild(new Node(words++));
			res->addChild(tmp);
			
		}
		else if(words->type==COMMA)
		{
			tmp->addChild(new Node(words));
			rescaned=true;
			goto rescan;
		}
		else
		{
			error(GRAMMARERR,"未识别标识符",words->line,words->col);
			find(words,SEMICOLON);
			if(words->type==ENDOFFILE)return NULL;
			words++;
			if(!inarr(words->type,2,INT,CHR))
				return res;
			else
				continue;
		}
	}
	return res;
}

static Node* getStatement(Word*& words)
{
	Node* res;
	Node* t;
	switch(words->type)
	{
	case IF:
		res=like(Statement,words,5,IF,LPAREN,Judgement,RPAREN,Statement);
		if(!res)
		{
			error(GRAMMARERR,"if语句定义有误",words[1].line,words[1].col);
			find(words,RPAREN);
			
			if(words->type==ENDOFFILE)return NULL;
			words++;
			return new Node(Statement,0);
		}
		t=like(Statement,words,2,ELSE,Statement);
		if(t)
			res->Merge(t);
		return res;
	case FOR:
		res=like(Statement,words,11,
			FOR,LPAREN,IDENTIFIER,ASSIGN,Expression,
			SEMICOLON,Judgement,SEMICOLON,IDENTIFIER,ASSIGN,
			IDENTIFIER);
		if(!res)
		{
			error(GRAMMARERR,"for语句定义有误",words[1].line,words[1].col);
			find(words,RPAREN);
			if(words->type==ENDOFFILE)return NULL;
			words++;
			return new Node(Statement,0);
		}
		if(!inarr(words->type,2,PLUS,MINUS))
		{
			error(GRAMMARERR,"if语句定义有误",words[0].line,words[0].col);
			find(words,RPAREN);
			
			if(words->type==ENDOFFILE)return NULL;
			words++;
			return new Node(Statement,0);
		}
		res->addChild(new Node(words++));
		t=like(Statement,words,3,UNSIGNED,RPAREN,Statement);
		if(!t)
		{
			error(GRAMMARERR,"if语句定义有误",words[1].line,words[1].col);
			find(words,RPAREN);
			
			if(words->type==ENDOFFILE)return NULL;
			words++;
			return new Node(Statement,0);
		}
		res->Merge(t);
		return res;
	case DO:
		res=like(Statement,words,6,DO,Statement,WHILE,LPAREN,Judgement,RPAREN);
		if(!res)
		{
			error(GRAMMARERR,"do语句定义有误",words[1].line,words[1].col);
			find(words,RPAREN);
			
			if(words->type==ENDOFFILE)return NULL;
			words++;
			return new Node(Statement,0);
		}
		return res;
	case LBRACE:
		res=new Node(Statement,0);
		res->addChild(new Node(words++));
		while(1)
		{
			if(words->type==RBRACE){
				res->addChild(new Node(words++));
				break;
			}
			t=getStatement(words);
			res->addChild(t);
		}
		return res;
	case IDENTIFIER:
		res=new Node(Statement,0);
		res->addChild(new Node(words++));
		if(words[0].type==LBRACHET)
		{
			//it's an array
			res->addChild(new Node(words++));
			Node* t=getExpression(words);
			if(!t)
			{

				find(words,SEMICOLON);
				
				if(words->type==ENDOFFILE)return NULL;
				words++;
				return res;
			}
			else
				res->addChild(t);
			if(words->type==RBRACHET)
				res->addChild(new Node(words++));
			else{
				error(GRAMMARERR,"数组调用有误",words->line,words->col);
				find(words,SEMICOLON);
				
				if(words->type==ENDOFFILE)return NULL;
				words++;
				return res;
			}

		}
		else if(words[0].type==LPAREN){
			res->addChild(new Node(words++));
			if(words->type==RPAREN)
			{
				res->addChild(new Node(words++));
			}
			else{
				while(1)
				{
					Node* t;
					if((t=getExpression(words))!=0)
						res->addChild(t);
					else
					{
						find(words,COMMA,RPAREN);
			if(words->type==ENDOFFILE)return NULL;
					}
					if(words->type==COMMA)
						res->addChild(new Node(words++));
					else if(words->type==RPAREN){
						res->addChild(new Node(words++));
						break;
					}
					else
					{
						error(GRAMMARERR,"函数调用有误",words->line,words->col);
						find(words,SEMICOLON);
			if(words->type==ENDOFFILE)return NULL;
						words++;
						return res;
					}
				}
			}
			if(words->type!=SEMICOLON)
			{
				error(GRAMMARERR,"语句未正确结束，是否少了分号？",words->line,words->col);
				find(words,SEMICOLON);
			if(words->type==ENDOFFILE)return NULL;
				words++;
				return res;
			}
			else
			{
				res->addChild(new Node(words++));
				return res;
			}
		}
		if(words->type!=ASSIGN)
		{
			error(GRAMMARERR,"赋值语句定义有误",words->line,words->col);
			find(words,SEMICOLON);
			if(words->type==ENDOFFILE)return NULL;
			words++;
			return res;
		}
		res->addChild(new Node(words++));
		t=getExpression(words);
		if(!t)
		{
			find(words,SEMICOLON);
			if(words->type==ENDOFFILE)return NULL;
			words++;
			return res;
		}
		res->addChild(t);
		if(words->type!=SEMICOLON)
		{
			error(GRAMMARERR,"赋值语句未正确结束,是否少了分号？",words->line,words->col);
			find(words,SEMICOLON);
			if(words->type==ENDOFFILE)return NULL;
			words++;
			return res;
		}
		res->addChild(new Node(words++));
		return res;
	case SCANF:
		res=new Node(Statement,0);
		res->addChild(new Node(words++));
		if(words->type==LPAREN){
			res->addChild(new Node(words++));
			while(1)
			{
				if(words->type==IDENTIFIER)
				{
					res->addChild(new Node(words++));
					if(words->type==COMMA)
					{
						res->addChild(new Node(words++));
						continue;
					}
					else if(words->type==RPAREN)
					{
						res->addChild(new Node(words++));
						
						break;
					}
					else
					{
						error(GRAMMARERR,"输入语句中的参数不正确",words->line,words->col);
						find(words,SEMICOLON);
			if(words->type==ENDOFFILE)return NULL;
						words++;
						return res;
					}
				}
				else
				{
					error(GRAMMARERR,"输入语句中的参数不正确",words->line,words->col);
					find(words,SEMICOLON);
			if(words->type==ENDOFFILE)return NULL;
					words++;
					return res;
				}
			}
		}
		else
		{
			error(GRAMMARERR,"输入语句语法错误",words->line,words->col);
			find(words,SEMICOLON);
			if(words->type==ENDOFFILE)return NULL;
			words++;
			return res;
		}
		if(words->type==SEMICOLON){
			res->addChild(new Node(words++));
			return res;
		}
		else
		{
			error(GRAMMARERR,"输入语句未正确结束,是否少了分号？",words->line,words->col);
			find(words,SEMICOLON);
			if(words->type==ENDOFFILE)return NULL;
			words++;
			return res;
		}
	case PRINTF:
		res=like(Statement,words,6,PRINTF,LPAREN,STRING,COMMA,Expression,RPAREN);
		if(!res)res=like(Statement,words,4,PRINTF,LPAREN,STRING,RPAREN);
		if(!res)res=like(Statement,words,4,PRINTF,LPAREN,Expression,RPAREN);
		if(!res)
		{
			error(GRAMMARERR,"输出语句中的参数不正确",words->line,words->col);
			find(words,SEMICOLON);
			if(words->type==ENDOFFILE)return NULL;
			words++;
			return new Node(Statement,0);
		}
		if(words->type==SEMICOLON){
			res->addChild(new Node(words++));
			return res;
		}
		else
		{
			error(GRAMMARERR,"输出语句未正确结束,是否少了分号？",words->line,words->col);
			find(words,SEMICOLON);
			if(words->type==ENDOFFILE)return NULL;
			words++;
			return res;
		}
	case RETURN:
		res=new Node(Statement,0);
		res->addChild(new Node(words++));
		if(words[0].type==SEMICOLON)
		{
			res->addChild(new Node(words++));
			return res;
		}
		t=like(Statement,words,3,LPAREN,Expression,RPAREN);
		if(t)
		{
			res->Merge(t);
			if(words->type==SEMICOLON){
				res->addChild(new Node(words++));
				return res;
			}
			else
			{
				error(GRAMMARERR,"输出语句未正确结束,是否少了分号？",words->line,words->col);
				find(words,SEMICOLON);
			if(words->type==ENDOFFILE)return NULL;
				words++;
				return res;
			}
		}
		else
		{
			error(GRAMMARERR,"返回语句语法错误",words->line,words->col);
			find(words,SEMICOLON);
			if(words->type==ENDOFFILE)return NULL;
			words++;
			return res;
		}
	case SEMICOLON:
		res=new Node(Statement,0);
		res->addChild(new Node(words++));
		return res;
	default:
		error(GRAMMARERR,"无法识别该语句",words->line,words->col);
		find(words,SEMICOLON);
			if(words->type==ENDOFFILE)return NULL;
		words++;
		return new Node(Statement,0);
	}
}
static Node* getComplex(Word*& words)
{
	Node* res=new Node(Complex,0);
	Node* t;
	if(words->type!=LBRACE)
	{
		error(GRAMMARERR,"复合语句有误，是否少了大括号？",words->line,words->col);
		find(words,SEMICOLON);
			if(words->type==ENDOFFILE)return NULL;
		words++;
	}
	res->addChild(new Node(words++));
	t=getConsts(words);
	res->addChild(t);
	t=getDeclares(words);
	res->addChild(t);
	while(1)
	{
		if(words->type==RBRACE)
		{
			res->addChild(new Node(words++));
			return res;
		}
		if((t=getStatement(words))==NULL)break;
		res->addChild(t);
	}
	return res;
}
static Node* getFunctions(Word*& words)
{
	Node* res=new Node(Functions,0);
	while(true)
	{
		Node* curr=new Node(Function,0);
		if(inarr(words->type,3,INT,CHR,VOID))
		{
			curr->addChild(new Node(words));
			if(words[1].type==IDENTIFIER)
			{
				curr->addChild(new Node(words+1));
				words+=2;
				if(words->type==LPAREN)
				{
					curr->addChild(new Node(words++));
					if(words->type!=RPAREN){
						while(true)
						{
						
							if(inarr(words->type,2,INT,CHR))
							{
								curr->addChild(new Node(words++));
								if(words->type==IDENTIFIER)
								{
									curr->addChild(new Node(words++));
								}
								else
								{
									error(GRAMMARERR,"函数参数名称定义有误",words->line,words->col);
									find(words,RPAREN,COMMA);
			if(words->type==ENDOFFILE)return NULL;
								}
								if(words->type==COMMA)
								{
									curr->addChild(new Node(words++));
									continue;
								}
								else if(words->type==RPAREN)
								{
									curr->addChild(new Node(words++));
									break;
								}
								else
								{
									error(GRAMMARERR,"函数参数定义有误",words->line,words->col);
									find(words,RPAREN);
			if(words->type==ENDOFFILE)return NULL;
									words++;
									break;
								}
							}
							else
							{
								error(GRAMMARERR,"函数参数类型定义有误",words->line,words->col);
								find(words,RPAREN);
			if(words->type==ENDOFFILE)return NULL;
								words++;
								break;
							}
						}
					}
					else
						curr->addChild(new Node(words++));
					Node* t=getComplex(words);
					curr->addChild(t);
					res->addChild(curr);
				}
			}
			else if(words[1].type==MAIN)
				return res;
		}
		else{
			error(GRAMMARERR,"无法识别标识符",words->line,words->col);
			find(words,INT,CHR,VOID);
			if(words->type==ENDOFFILE)return NULL;
		}
	}
}
static Node* getMain(Word*& words)
{
	Node* res=like(Main,words,4,VOID,MAIN,LPAREN,RPAREN);
	if(!res)
	{
		error(GRAMMARERR,"函数定义有误",words->line,words->col);
		find(words,LBRACE);
			if(words->type==ENDOFFILE)return NULL;
	}
	Node* cpx=getComplex(words);
	if(res!=NULL)
		res->addChild(cpx);
	return res;
}
//return null if error happend at the end of file
static Node* getProgram(Word*& words)
{
	Node* res=new Node(Program,0);
	res->addChild(getConsts(words));
	res->addChild(getDeclares(words));
	res->addChild(getFunctions(words));
	res->addChild(getMain(words));
	if(words->type==ENDOFFILE)
	{
		return res;
	}
	error(GRAMMARERR,"无法识别标识符",words->line,words->col);
	return NULL;
}
Node* getNodeFrom(NodeType type, Word*& words)
{
	switch (type)
	{
	case Number:
		return getNumber(words);
	case Operator:
		return getOperator(words);
	case Consts:
		return getConsts(words);
	case Declarations:
		return getDeclares(words);
	case Factor:
		return getFactor(words);
	case Term:
		return getTerm(words);
	case Expression:
		return getExpression(words);
	case Judgement:
		return getJudgement(words);
	case Statement:
		return getStatement(words);
	case Complex:
		return getComplex(words);
	case Functions:
		return getFunctions(words);
	case Main:
		return getMain(words);
	case Program:
		return getProgram(words);
	default:
		return NULL;
	}
}