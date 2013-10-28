#include "word.h"
#include "string.h"
#include <stdio.h>
char SPACE[4]={' ','\t','\r','\n'};
int SPACEC=4;
char DOP[3]={'<','>','='};
int DOPC=3;
char SOP[]={',',';','(',')','[',']','{','}','+','-','*','/'};
int SOPC=12;
int currcol=0,currline=1;
//0:idle, 
//1:start with a letter, 
//2:start with a number, 
//3:start with a ", 
//4:start with a ',
//5:other:must be a reserved word or error
int state=0;
//return SPLIT if current word isn't a reserved word
WordType search(char* str)
{
	//a simple algorithm
	for(int i=0;i<RESERVED_WORDS_COUNT;i++)
		if(strcmp(str,RESERVED_WORDS[i])==0)
			return (WordType)(i+1);
	return SPLIT;
}
char* subString(char* begin,char* end)
{
	char* res=new char[end-begin+2];
	memcpy(res,begin,end-begin+1);
	res[end-begin+1]=0;
	return res;
}
char* tryGetWord(char* str,Word* res)
{
	char ch;
	res->line=currline;
	res->col=currcol;
	while(str[0]!=0&&inarr(ch=*str,SPACE,SPACEC)){
		if(ch=='\n')
		{
			res->col=currcol=0;
			res->line=(++currline);
		}
		else
			res->col=(++currcol);
		str++;
	}
	if(str[0]==0)return 0;
	if(ch=='"')//a string
	{
		int i;
		for(i=1;str[i]!=0;i++){
			if(str[i]=='"')
			{
				res->type=STRING;
				res->word=subString(str+1,str+i-1);
				res->col=currcol+=i+1;
				return str+i+1;
			}
			else{
				if(str[i]=='\n'){
					res->col=currcol=0;
					res->line=(++currline);
					i++;
					break;
				}
			}
		}
		error(WORDERR,"字符串定义有误",currline,currcol);
		return str+i;
	}
	else if(ch=='\'')//a char
	{
		char op[]={'+','-','*','/'};
		if((inarr(str[1],op,4)||in(str[1],'a','z')||in(str[1],'A','Z')||in(str[1],'0','9')||str[1]=='_'))
		{
			if(str[2]=='\'')
			{
				res->type=CHAR;
				res->word=subString(str+1,str+1);
				res->col=(currcol+=3);
				return str+3;
			}
		}
		error(WORDERR,"字符定义有误",currline,currcol);
		
		int i;
		for(i=0;str[i]!=0&&str[i]!=';';i++){
			if(str[i]=='\n'){
				res->line=(++currline);
				res->col=currcol=0;
			}
			else
				res->col=(++currcol);
		}
		return str+i;
	}
	else if(in(ch,'a','z')||in(ch,'A','Z')||ch=='_')
	{
		//must be a identifier or reserved word
		for(int i=1;;i++)
		{
			if(!(in(str[i],'a','z')||in(str[i],'A','Z')||str[i]=='_'||in(str[i],'0','9')))
			{
				res->word=subString(str,str+i-1);
				if((res->type=search(res->word))==SPLIT)
					res->type=IDENTIFIER;
				res->col=(currcol+=i);
				return str+i;
			}
		}
	}
	else if(in(ch,'1','9'))
	{
		//must be a number
		for(int i=1;;i++)
		{
			if(!(in(str[i],'0','9')))
			{
				res->type=UNSIGNED;
				res->word=subString(str,str+i-1);
				res->col=(currcol+=i);
				return str+i;
			}
		}
	}
	else if(ch=='0')
	{
		//must be zero
		res->type=ZERO;
		res->word=subString(str,str);
		res->col=(++currcol);
		return str+1;
	}
	else{
		//must be a reserved word
		if(inarr(ch,SOP,SOPC))
		{
			res->word=subString(str,str);
			res->type=search(res->word);
			res->col=(++currcol);
			return str+1;
		}
		else if(inarr(ch,DOP,DOPC))
		{
			if(str[1]=='=')
			{
				res->word=subString(str,str+1);
				res->type=search(res->word);
				res->col=(currcol+=2);
				return str+2;

			}
			else
			{
				res->word=subString(str,str);
				res->type=search(res->word);
				res->col=(++currcol);
				return str+1;
			}
		}
		else if(ch=='!'&&str[1]=='=')
		{
			res->word=subString(str,str+1);
			res->type=NET;
			res->col=(currcol+=2);
			return str+2;
		}
		error(WORDERR,"无法识别符号",currline,currcol);
		int i;
		for(i=2;str[i]!=0&&str[i]!=';';i++){
			if(str[i]=='\n'){
				res->line=(++currline);
				res->col=currcol=0;
			}
			else
				res->col=(++currcol);
		}
		return str+i;
	}
}

void outputDemo(char* content)
{
	Word* w=new Word(SPLIT,0,0);
	int i=0;
	while((content=tryGetWord(content,w))!=0)
	{
		switch (w->type)
		{
		case IDENTIFIER:
			printf("%d IDEN %s\n",++i,w->word);
			break;
		case UNSIGNED:
			printf("%d INTCON %s\n",++i,w->word);
			break;
		case CHAR:
			printf("%d CHARCON %s\n",++i,w->word);
			break;
		case STRING:
			printf("%d STRCON %s\n",++i,w->word);
			break;
		case CONST:
			printf("%d CONSTTK %s\n",++i,w->word);
			break;
		case INT:
			printf("%d INTTK %s\n",++i,w->word);
			break;
		case CHR:
			printf("%d CHARTK %s\n",++i,w->word);
			break;
		case VOID:
			printf("%d VOIDTK %s\n",++i,w->word);
			break;
		case MAIN:
			printf("%d MAINTK %s\n",++i,w->word);
			break;
		case IF:
			printf("%d IFTK %s\n",++i,w->word);
			break;
		case DO:
			printf("%d DOTK %s\n",++i,w->word);
			break;
		case ELSE:
			printf("%d ELSETK %s\n",++i,w->word);
			break;
		case WHILE:
			printf("%d WHILETK %s\n",++i,w->word);
			break;
		case FOR:
			printf("%d FORTK %s\n",++i,w->word);
			break;
		case SCANF:
			printf("%d SCANFTK %s\n",++i,w->word);
			break;
		case PRINTF:
			printf("%d PRINTFTK %s\n",++i,w->word);
			break;
		case RETURN:
			printf("%d RETURNTK %s\n",++i,w->word);
			break;
		case PLUS:
			printf("%d PLUS %s\n",++i,w->word);
			break;
		case MINUS:
			printf("%d MINU %s\n",++i,w->word);
			break;
		case MULTIPLY:
			printf("%d MULT %s\n",++i,w->word);
			break;
		case DIVIDE:
			printf("%d DIV %s\n",++i,w->word);
			break;
		case LT:
			printf("%d LSS %s\n",++i,w->word);
			break;
		case LTET:
			printf("%d LEQ %s\n",++i,w->word);
			break;
		case RT:
			printf("%d GRE %s\n",++i,w->word);
			break;
		case RTET:
			printf("%d GEQ %s\n",++i,w->word);
			break;
		case ET:
			printf("%d EQL %s\n",++i,w->word);
			break;
		case NET:
			printf("%d NEQ %s\n",++i,w->word);
			break;
		case ASSIGN:
			printf("%d ASSIGN %s\n",++i,w->word);
			break;
		case SEMICOLON:
			printf("%d SEMICN %s\n",++i,w->word);
			break;
		case COMMA:
			printf("%d COMMA %s\n",++i,w->word);
			break;
		case SINGLEQUOTE:
			printf("%d QMARK %s\n",++i,w->word);
			break;
		case DOUBLEQUOTE:
			printf("%d DQMARK %s\n",++i,w->word);
			break;
		case LPAREN:
			printf("%d LPARENT %s\n",++i,w->word);
			break;
		case RPAREN:
			printf("%d RPARENT %s\n",++i,w->word);
			break;
		case LBRACHET:
			printf("%d LBRACK %s\n",++i,w->word);
			break;
		case RBRACHET:
			printf("%d RBRACK %s\n",++i,w->word);
			break;
		case LBRACE:
			printf("%d LBRACE %s\n",++i,w->word);
			break;
		case RBRACE:
			printf("%d RBRACE %s\n",++i,w->word);
			break;
		default:
			break;
		}
	}
}