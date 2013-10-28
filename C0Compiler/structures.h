#ifndef STRUCTURE_H
#define STRUCTURE_H
#include "compiler.h"
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
enum WordType{
	//[ \t\r\n]
	SPLIT=0,
	//const
	CONST=1,
	//int
	INT=2,
	//char
	CHR=3,
	//+
	PLUS=4,
	//-
	MINUS=5,
	//*
	MULTIPLY=6,
	// /
	DIVIDE=7,
	//=
	ASSIGN=8,
	//==
	ET=9,
	//<
	LT=10,
	//<=
	LTET=11,
	//>
	RT=12,
	//>=
	RTET=13,
	//!=
	NET=14,
	//void
	VOID=15,
	//if
	IF=16,
	//else
	ELSE=17,
	//do
	DO=18,
	//while
	WHILE=19,
	//(
	LPAREN=20,
	//)
	RPAREN=21,
	//[
	LBRACHET=22,
	//]
	RBRACHET=23,
	//{
	LBRACE=24,
	//}
	RBRACE=25,
	//for
	FOR=26,
	//;
	SEMICOLON=27,
	//,
	COMMA=28,
	//'
	SINGLEQUOTE=29,
	//"
	DOUBLEQUOTE=30,
	//scanf
	SCANF=31,
	//printf
	PRINTF=32,
	//return
	RETURN=33,
	//main
	MAIN=34,
	//0
	ZERO=35,
	
	//[_a-zA-Z][_a-zA-Z0-9]*
	IDENTIFIER=41,
	//[1-9][0-9]*
	UNSIGNED=42,
	//<|<=|>|>=|!=|==
	OPERATOR=43,
	//"..."
	STRING=44,
	//'.'
	CHAR=45,

	ENDOFFILE=100
};
const char RESERVED_WORDS[][MAXWL]={
	"const",
	"int",
	"char",
	"+",
	"-",
	
	"*",
	"/",
	"=",
	"==",
	"<",

	"<=",
	">",
	">=",
	"!=",
	"void",

	"if",
	"else",
	"do",
	"while",
	"(",

	")",
	"[",
	"]",
	"{",
	"}",

	"for",
	";",
	",",
	"'",
	"\"",

	"scanf",
	"printf",
	"return",
	"main",
	"0"
};
const int RESERVED_WORDS_COUNT=35;

struct Word
{
	WordType type;
	char* word;
	Word(WordType type,char* str,int len){
		this->type=type;
		word=str;
	}
	Word(){
		word=0;
	}
	int col;
	int line;
	~Word(){if(word)delete word;word=0;}
	int parseInt()
	{
		assert(this->type==UNSIGNED||type==ZERO);
		return atoi(this->word);
	}
};
#define isSameWord(w1,w2) ((w1).type==(w2).type&&strcmp((w1).word,(w2).word)==0)
enum NodeType
{
	Number=101,
	Operator=102,
	Const=103,
	Consts=104,
	Factor=105,
	Term=106,
	Expression=107,
	Judgement=108,
	Statement=109,
	Declaration=110,
	Declarations=111,
	Complex=112,
	Function=113,
	Functions=114,
	Main=115,
	Program=116,
};
struct Node
{
#define  MAXNODE 2000
	bool isWord;
	NodeType nodeType;
	Node* childList[MAXNODE];
	int childCount;
	Word* word;
	struct{int col,line;} position;
	int otherInfo;
	Node(NodeType type,int count,...)
	{
		this->nodeType=type;
		va_list ap;
		isWord=false;
		childCount=count;
		va_start(ap,count);
		for(int i=0;i<count;i++){
			
			childList[i]=va_arg(ap,Node*);
			if(i==0)
			{
				position=childList[0]->position;
			}
		}
		va_end(ap);
	}
	Node(Word* wd,int info=0)
	{
		this->word=wd;
		this->isWord=true;
		this->position.col=wd->col;
		this->position.line=wd->line;
	}
	bool addChild(Node* nd)
	{
		if(isWord)return false;
		if(childCount>=MAXNODE)return false;
		childList[childCount++]=nd;
		if(childCount==1)
			this->position=nd->position;
		return true;
	}
	bool Merge(Node* nd)
	{
		if(!nd)return false;
		for(int i=0;i<nd->childCount;i++)
			addChild(nd->childList[i]);
		return true;
	}
	//if this node is a number or unsigned, return the int
	int parseInt()
	{
		assert(this->nodeType==Number||this->isWord);
		if(this->isWord)
			return this->word->parseInt();
		if(this->childCount==1)
			return childList[0]->word->parseInt();
		else
		{
			if(this->childList[0]->word->type==MINUS)
				return -this->childList[1]->word->parseInt();
			else
				return childList[0]->word->parseInt();
		}
	}
};
struct Table{
#define MAXREC 1000
#define MAXLAYER 3
	struct Record{
#define MAXPARAM 20
		char* name;
		enum{
			Constance,
			Variable,
			Procedure
		} identType;
		struct VarType{
			WordType type;
			bool isArray;
			int arrlength;
		} varType;
		int startAddress;
		bool isGlobal;
		char* getDest(char* buff)
		{
			assert(this->varType.isArray==false);
			if(reg!=NOREG)
			{
				switch (reg)
				{
				case EBX:
					sprintf(buff,"EBX");
					return buff;
					
				case ECX:
					sprintf(buff,"ECX");
					return buff;
					
				case ESI:
					sprintf(buff,"ESI");
					return buff;
					
				case EDI:
					sprintf(buff,"EDI");
					return buff;

				default:
					break;
				}
			}
			if(isGlobal)
			{
				sprintf(buff,"[__base+%d]",startAddress);
				return buff;
			}
			else
			{
				sprintf(buff,"[EBP+%d]",-startAddress);
				return buff;
			}
		}
		/*Record* index;
		Record* parentArray;
		bool isArrayPtr()
		{
			return index!=0;
		}*/
		enum Reg{
			NOREG=0,
			EBX=1,
			ECX=2,
			ESI=3,
			EDI=4,
			REG_MAX=5
		} reg;
		//always return 4*arrlength
		int getSize()
		{
			if(this->identType==Procedure)return 0;
			//if(this->varType.type==INT)
			return varType.isArray?(4*varType.arrlength):4;
			//else if(this->varType.type==CHR)
			//	return varType.isArray?varType.arrlength:1;
			//return 0;
		}
		Record* parameters[MAXPARAM];
		WordType parametersType[MAXPARAM];
		int parameterCount;
		/*union{
			char* contentStr;
			int contentInt;
			char contentChar;
		} value;*/
		Record()
		{
			name=0;
			identType=Variable;
			varType.isArray=false;
			varType.type=INT;
			startAddress=0;
			//index=0;
			reg=NOREG;
			parameterCount=0;
			isGlobal=false;
			imme=false;
			//parentArray=NULL;
		}
		bool imme;
		int immenum;
	};
	Record* recs[MAXREC];
	int recsCount;
	int Layers[MAXLAYER];
	int currLayer;
};
struct Quaternion
{
	enum Command{
		Q_ADD, //add rec,rec,rec
		Q_ADD_C, //add_c rec,int,rec
		Q_SUB, //sub rec,rec,rec
		Q_SUB_C,//sub_c rec,int ,rec
		Q_MUL,//mul rec,rec,rec
		Q_DIV,//div rec,rec,rec
		Q_OPP,//opp rec,null,rec

		Q_ET,//et rec,rec,label(rec)
		Q_LT,//lt rec,rec,label(rec)
		Q_LTET,
		Q_RTET,
		Q_RT,
		Q_NET,
		Q_NET_0,//net rec,null,label(rec)

		Q_ARRR,//arrr rec(array),rec(index),rec(dest)
		Q_ARRW,//arrw rec(array),rec(index),rec(source)
		Q_ASSIGN,//assign rec,rec

		Q_LABEL,//label label(string constant)

		Q_ASSIGNPROC,//assignproc rec(procname)

		Q_RET,//ret rec
		Q_RET_N,//ret

		Q_ALLOC,//alloc rec
		Q_ALLOC_C, //alloc_C rec, INT/CHR constant

		Q_PUSH,//push rec

		Q_CALL,//call procname(string constant),rec(return dest)
		Q_CALL_N,//call procname(string constant)

		Q_JMP,//jmp label

		Q_PRINTSTR,//printstr rec(string constant)
		Q_PRINTINT,//printint rec(int not constant)
		Q_PRINTCHAR,//printchar rec(char not constant)

		Q_INPUT,//input rec(dest)

		Q_GLOBAL,//global rec
		Q_GLOBAL_C, //global_c rec, int
		Q_ENDGLOBAL, //endglobal
		Q_TERMINATE //terminate
	} command;
	
	union{
		char* contentStr;
		int contentInt;
		char contentChar;
		Quaternion* label;
		Table::Record* record;
	} param[3];
	static int _lbl_count;
	static char* createLabel()
	{
		char* res=new char[10];
		sprintf(res,"lbl%d",_lbl_count++);
		return res;
	}
};
#endif