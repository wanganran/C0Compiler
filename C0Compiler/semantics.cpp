#include "semantics.h"
#include "table.h"
int Quaternion::_lbl_count=0;
#define createJmp(jmp,lbl) {jmp=addQuaternion();jmp->command=Quaternion::Q_JMP;jmp->param[0].label=lbl;}
#define createLbl(lbl) {lbl=addQuaternion();lbl->command=Quaternion::Q_LABEL;lbl->param[0].contentStr=Quaternion::createLabel();}
#define create(q,type) {q=addQuaternion();q->command=type;}
static WordType funcRetType=VOID;
static Quaternion* base,*ptr;

static Quaternion* addQuaternion()
{
	return ptr++;
}
static void makeConsts(Node* consts,bool global=false)
{
	assert(consts->nodeType==Consts);
	for(int i=0;i<consts->childCount;i++)
	{
		Node* nd=consts->childList[i];
		assert(nd->nodeType==Const);
		int c=2;
		while(c<nd->childCount){
			if(hasSym(nd->childList[c]->word->word))
			{
				error(SEMANTICSERR,"常量名称重定义",nd->childList[c]->position.line,
					nd->childList[c]->position.col);
				c+=4;
				continue;
			}
			Table::Record* rec=addSym();
			rec->name=nd->childList[c]->word->word;
			rec->identType=Table::Record::Constance;
			rec->isGlobal=global;
			rec->varType.type=nd->childList[1]->word->type;
			rec->varType.isArray=false;
			Quaternion* currq=addQuaternion();
			
			switch(nd->childList[1]->word->type)
			{
			case INT:
				currq->param[1].contentInt=nd->childList[c+2]->parseInt();
				break;
			case CHR:
				currq->param[1].contentChar=nd->childList[c+2]->word->word[0];
				break;
			}
			currq->command=global?Quaternion::Q_GLOBAL_C:Quaternion::Q_ALLOC_C;
			currq->param[0].record=rec;
			c+=4;
		}
	}
}
static void makeVaribles(Node* declares,bool global=false)
{
	assert(declares->nodeType==Declarations);
	for(int i=0;i<declares->childCount;i++)
	{
		Node* nd=declares->childList[i];
		assert(nd->nodeType==Declaration);
		int c=1;
		while(c<nd->childCount){
#define findnext while(!(nd->childList[c]->isWord&&inarr(nd->childList[c]->word->type,2,COMMA,SEMICOLON)))c++;c++;
			if(hasSym(nd->childList[c]->word->word))
			{
				error(SEMANTICSERR,"变量名称重定义",nd->childList[c]->position.line,nd->childList[c]->position.col);
				findnext;
				continue;
			}
			Table::Record* rec=addSym();
			rec->name=nd->childList[c]->word->word;
			rec->identType=Table::Record::Variable;
			rec->isGlobal=global;
			rec->varType.type=nd->childList[0]->word->type;
			if(nd->childList[c+1]->isWord&&nd->childList[c+1]->word->type==LBRACHET)
			{
				rec->varType.isArray=true;
				rec->varType.arrlength=nd->childList[c+2]->parseInt();
				c+=4;
			}
			else
			{
				rec->varType.isArray=false;
				c+=1;
			}
			c++;
			
			Quaternion* currq=addQuaternion();
			currq->command=global?Quaternion::Q_GLOBAL:Quaternion::Q_ALLOC;
			currq->param[0].record=rec;
		}
	}
}
static Table::Record* makeExpression(Node*);
static Table::Record* makeFactor(Node* factor)
{
	int i;
	Quaternion* currq;
	Table::Record* rec,*ident;
	if(!factor->childList[0]->isWord)
	{
		//must be a number
		rec=addTmpSym();
		create(currq,Quaternion::Q_ALLOC_C);
		currq->param[0].record=rec;
		currq->param[1].contentInt=factor->childList[0]->parseInt();
		rec->imme=true;
		rec->immenum=factor->childList[0]->parseInt();
		return rec;
		
	}
	else
	{
		switch (factor->childList[0]->word->type)
		{
		case CHAR:
			rec=addTmpSym();
			rec->varType.type=CHR;

			create(currq,Quaternion::Q_ALLOC_C);
			currq->param[0].record=rec;
			currq->param[1].contentChar=factor->childList[0]->word->word[0];
			
			return rec;
		case IDENTIFIER:
			if((ident=getSym(factor->childList[0]->word->word))==NULL)
			{
				error(SEMANTICSERR,"未声明标识符",factor->childList[0]->position.line,
					factor->childList[0]->position.col);
				return NULL;
			}
			if(factor->childCount>1)
			{
				if(factor->childList[1]->word->type==LBRACHET)
				{
					//it's a pointer
					if(!ident->varType.isArray)
						error(SEMANTICSERR,"对非数组进行下标访问",factor->childList[1]->position.line,
							factor->childList[1]->position.col);
					Table::Record* index=makeExpression(factor->childList[2]);
					if(index->imme&&(index->immenum>=ident->varType.arrlength||index->immenum<0))
						error(SEMANTICSERR,"对数组的索引超出了数组的界限",factor->childList[2]->position.line,factor->childList[2]->position.col);
					Table::Record* rec=addTmpSym();
					rec->varType.type=ident->varType.type;
					create(currq,Quaternion::Q_ALLOC);
					currq->param[0].record=rec;
						
					create(currq,Quaternion::Q_ARRR);
					currq->param[0].record=ident;
					currq->param[1].record=index;
					currq->param[2].record=rec;
					return rec;
				}
				else if(factor->childList[1]->word->type==LPAREN)
				{
					//it's a function
					/*if(!(ident=getSym(factor->childList[0]->word->word)))
					{
						error(SEMANTICSERR,"未声明标识符",factor->childList[0]->position.line,
							factor->childList[0]->position.col);
					}*/

					if(ident->identType!=Table::Record::Procedure)
					{
						error(SEMANTICSERR,"标识符不是函数",factor->childList[0]->position.line,
							factor->childList[0]->position.col);
					}
					else{
					if(ident->varType.type==VOID){
						error(SEMANTICSERR,"该函数不返回值",factor->childList[0]->position.line,
							factor->childList[0]->position.line);
					}
					else if(!(factor->childCount==3&&ident->parameterCount==0)&&(
						factor->childCount-2!=ident->parameterCount*2))
					{
						error(SEMANTICSERR,"函数参数数量不正确",factor->childList[2]->position.line,
							factor->childList[2]->position.col);
					}
					}
					Table::Record* pushes[MAXREC];
					int pushescount=0;
					for(i=0;i<factor->childCount/2-1;i++)
					{
						Table::Record* rec=makeExpression(factor->childList[i*2+2]);
						if(ident->parameterCount<=i)continue;
						if((rec->varType.type==INT&&ident->parametersType[i]==CHR))
						{
							error(SEMANTICSERR,"函数参数类型不正确",factor->childList[i*2+2]->position.line,
								factor->childList[i*2+2]->position.col);
						}
						pushes[pushescount++]=rec;
						//currq->paramType[0]=Quaternion::Record;
					}
					
					Table::Record* t=addTmpSym();
					create(currq,Quaternion::Q_ALLOC);
					currq->param[0].record=t;
					for(i=0;i<pushescount;i++)
					{
						create(currq,Quaternion::Q_PUSH);
						currq->param[0].record=pushes[i];
					}
					create(currq,Quaternion::Q_CALL);
					
					currq->param[0].contentStr=ident->name;
					currq->param[1].record=t;
					return t;
				}
			}
			else
			{
				/*if(!(ident=getSym(factor->childList[0]->word->word)))
				{
					error(SEMANTICSERR,"未声明标识符",factor->childList[0]->position.line,
						factor->childList[0]->position.col);
				}*/
				if(ident->varType.isArray)
					error(SEMANTICSERR,"只有整型或字符型可以参与运算",factor->childList[0]->position.line,
						factor->childList[0]->position.col);
				if(ident->varType.type==STRING)
					error(SEMANTICSERR,"只有整型或字符型可以参与运算",factor->childList[0]->position.line,
						factor->childList[0]->position.col);
				return ident;
			}
		case LPAREN:
			return makeExpression(factor->childList[1]);
		default:
			return NULL;
		}
	}
}
static Table::Record* makeTerm(Node* term)
{
	assert(term->nodeType==Term);
	Quaternion* currq;
	Table::Record* rec=makeFactor(term->childList[0]);
	if(term->childCount>1){
		Table::Record* nrec=addTmpSym();
		create(currq,Quaternion::Q_ALLOC);
		currq->param[0].record=nrec;
		create(currq,Quaternion::Q_ASSIGN);
		currq->param[0].record=nrec;
		currq->param[1].record=rec;
		rec=nrec;
		rec->imme=false;
	}
	for(int i=1;i<term->childCount;i+=2)
	{
		Table::Record* t=makeFactor(term->childList[i+1]);
		if(term->childList[i]->word->type==MULTIPLY)
		{
			create(currq,Quaternion::Q_MUL);
		}
		else
			create(currq,Quaternion::Q_DIV);
		currq->param[0].record=rec;
		currq->param[1].record=t;
		currq->param[2].record=rec;
	}
	return rec;
}
static Table::Record* makeExpression(Node* statement)
{
	Quaternion* currq;
	Table::Record* t,*t2;
	int i=0;
	if(statement->childList[0]->isWord)
	{
		//must be + or -
		t=makeTerm(statement->childList[1]);
		
		if(statement->childList[0]->word->type==MINUS)
		{
			create(currq,Quaternion::Q_OPP);
			currq->param[0].record=t;
			currq->param[2].record=t;
			if(t->imme)
				t->immenum=-t->immenum;
		}
		i=1;
	}
	else
		t=makeTerm(statement->childList[0]);
	i++;
	if(statement->childCount>i){
		Table::Record* nrec=addTmpSym();
		create(currq,Quaternion::Q_ALLOC);
		currq->param[0].record=nrec;
		create(currq,Quaternion::Q_ASSIGN);
		currq->param[0].record=nrec;
		currq->param[1].record=t;
		t=nrec;
		t->imme=false;
	}
	for(;i<statement->childCount-1;i+=2)
	{
		t2=makeTerm(statement->childList[i+1]);
		if(statement->childList[i]->word->type==PLUS)
		{
			create(currq,Quaternion::Q_ADD);
		}
		else
		{
			create(currq,Quaternion::Q_SUB);
		}
		currq->param[0].record=t;
		currq->param[1].record=t2;
		currq->param[2].record=t;

	}
	return t;
}
static Quaternion* makeJudgement(Node* judgement)
{
	assert(judgement->nodeType==Judgement);
		
	Quaternion* currq;
	if(judgement->childCount==1){
		Table::Record* t1=makeExpression(judgement->childList[0]);
		currq=addQuaternion();
		currq->command=Quaternion::Q_NET_0;
		currq->param[0].record=t1;
		//currq->paramType[0]=Quaternion::Record;
	}
	else
	{
		Table::Record* t1=makeExpression(judgement->childList[0]);
		Table::Record* t2=makeExpression(judgement->childList[2]);
		currq=addQuaternion();
		switch (judgement->childList[1]->childList[0]->word->type)
		{
		case LT:
			currq->command=Quaternion::Q_LT;
			break;
		case RT:
			currq->command=Quaternion::Q_RT;
			break;
		case LTET:
			currq->command=Quaternion::Q_LTET;
			break;
		case RTET:
			currq->command=Quaternion::Q_RTET;
			break;
		case ET:
			currq->command=Quaternion::Q_ET;
			break;
		case NET:
			currq->command=Quaternion::Q_NET;
			break;
		}
		currq->param[0].record=t1;
		currq->param[1].record=t2;
		//currq->paramType[0]=currq->paramType[1]=Quaternion::Record;
	}
	return currq;
}
static void makeStatement(Node* statement)
{
	assert(statement->nodeType==Statement);
	Quaternion *currq,*jmp,*lbl,*delta;
	Table::Record* ident1,*dest,*t1,*exp;
	int count,i,j,k;
	//FUCKING VC6
	//FUCKING VC6
	//FUCKING VC6
	//FUCKING VC6
	switch (statement->childList[0]->word->type)
	{
	case IF:
		currq=makeJudgement(statement->childList[2]);
		makeStatement(statement->childList[4]);

		if(statement->childCount>5)
		{
			//there exist ELSE
			createJmp(jmp,NULL);
			createLbl(lbl);
			currq->param[2].label=lbl;
			makeStatement(statement->childList[6]);
			createLbl(lbl);
			jmp->param[0].label=lbl;
		}
		else
		{
			createLbl(lbl);
			currq->param[2].label=lbl;
		}
		break;
	case FOR:
		//indet=exp
		if((ident1=getSym(statement->childList[2]->word->word))==NULL)
		{
			error(SEMANTICSERR,"未声明标识符",statement->childList[2]->position.line,
				statement->childList[2]->position.col);
		}
		else if(ident1->identType!=Table::Record::Variable)
		{
			error(SEMANTICSERR,"只能对变量赋值",statement->childList[2]->position.line,
				statement->childList[2]->position.col);
		}
		else if(ident1->varType.isArray)
		{
			error(SEMANTICSERR,"不能对数组赋值",statement->childList[2]->position.line,
				statement->childList[2]->position.col);
		}
		t1=makeExpression(statement->childList[4]);
		currq=addQuaternion();
		currq->command=Quaternion::Q_ASSIGN;
		currq->param[0].record=ident1;
		currq->param[1].record=t1;
		//currq->paramType[1]=Quaternion::Record;
		
		createLbl(lbl);
		//judgement
		currq=makeJudgement(statement->childList[6]);
		//statement
		makeStatement(statement->childList[14]);
		//delta
		delta=addQuaternion();	
		if(statement->childList[11]->word->type==PLUS)
		{
			delta->command=Quaternion::Q_ADD_C;
		}
		else
		{
			delta->command=Quaternion::Q_SUB_C;
		}
		if((ident1=getSym(statement->childList[10]->word->word))==NULL)
		{
			error(SEMANTICSERR,"未声明标识符",statement->childList[10]->position.line,
				statement->childList[10]->position.col);
		}
		else if(ident1->identType==Table::Record::Procedure||
			ident1->varType.isArray||
			!inarr(ident1->varType.type,2,INT,CHR))
		{
			error(SEMANTICSERR,"只有整型或字符型可以参与运算",statement->childList[10]->position.line,
				statement->childList[10]->position.col);
		}
		delta->param[0].record=ident1;
		delta->param[1].contentInt=statement->childList[12]->parseInt();
		//delta->paramType[0]=Quaternion::Record;
		//delta->paramType[1]=Quaternion::ContentInt;
		if((ident1=getSym(statement->childList[8]->word->word))==NULL)
		{
			error(SEMANTICSERR,"未声明标识符",statement->childList[8]->position.line,statement->childList[8]->position.col);
		}
		else if(ident1->identType!=Table::Record::Variable)
		{
			error(SEMANTICSERR,"只能对变量赋值",statement->childList[8]->position.line,
				statement->childList[2]->position.col);
		}
		else if(ident1->varType.isArray)
		{
			error(SEMANTICSERR,"不能对数组赋值",statement->childList[8]->position.line,
				statement->childList[2]->position.col);
		}
		delta->param[2].record=ident1;
		//jmp to start
		createJmp(jmp,lbl);
		//over
		createLbl(lbl);
		currq->param[2].label=lbl;
		break;

	case DO:
		createLbl(lbl);
		makeStatement(statement->childList[1]);
		currq=makeJudgement(statement->childList[4]);
		createJmp(jmp,lbl);
		createLbl(lbl);
		currq->param[2].label=lbl;
		break;

	case LBRACE:
		for(i=1;i<statement->childCount-1;i++)
		{
			makeStatement(statement->childList[i]);
		}
		break;

	case IDENTIFIER:
		if(statement->childList[1]->word->type==LBRACHET)
		{
			//an array pointer
			Table::Record* index=makeExpression(statement->childList[2]);
			
			Table::Record* arr;
			if(!(arr=getSym(statement->childList[0]->word->word)))
			{
				error(SEMANTICSERR,"未声明标识符",statement->childList[0]->position.line,statement->childList[0]->position.col);
			}
			else if(arr->varType.isArray==false)
			{
				error(SEMANTICSERR,"对非数组进行下标访问",statement->childList[0]->position.line,statement->childList[0]->position.col);
			}
			else
			{
				if(index->imme)
				{
					if(index->immenum>=arr->varType.arrlength||index->immenum<0)
						error(SEMANTICSERR,"对数组的索引超出了数组的界限",statement->childList[2]->position.line,statement->childList[2]->position.col);
				}
				exp=makeExpression(statement->childList[5]);
				Table::Record* exp2=makeExpression(statement->childList[2]);
				create(currq,Quaternion::Q_ARRW);
				currq->param[0].record=arr;
				currq->param[1].record=exp2;
				currq->param[2].record=exp;
				break;
			}
		}
		else if(statement->childList[1]->word->type==ASSIGN)
		{
			if(!(dest=getSym(statement->childList[0]->word->word)))
			{
				error(SEMANTICSERR,"未声明标识符",statement->childList[0]->position.line,statement->childList[0]->position.col);
			}
			else if(dest->varType.isArray)
			{
				error(SEMANTICSERR,"对数组只能用下标进行访问",statement->childList[0]->position.line,statement->childList[0]->position.col);
			}
			else if(dest->identType!=Table::Record::Variable)
			{
				error(SEMANTICSERR,"只能对变量进行赋值操作",statement->childList[0]->position.line,statement->childList[0]->position.col);
			
			}
			exp=makeExpression(statement->childList[2]);
			create(currq,Quaternion::Q_ASSIGN);
			currq->param[0].record=dest;
			currq->param[1].record=exp;
			break;
		}
		else if(statement->childList[1]->word->type==LPAREN)
		{
			if(!(dest=getSym(statement->childList[0]->word->word)))
			{
				error(SEMANTICSERR,"未声明标识符",statement->childList[0]->position.line,statement->childList[0]->position.col);
				return;
			}

			else if(dest->identType!=Table::Record::Procedure)
			{
				error(SEMANTICSERR,"标识符不是函数",statement->childList[0]->position.line,statement->childList[0]->position.col);
				return;
			}
			else if(!(statement->childCount==4&&dest->parameterCount==0)&&
				(statement->childCount-3!=dest->parameterCount*2))
			{
				error(SEMANTICSERR,"函数参数数量不正确",statement->childList[2]->position.line,statement->childList[2]->position.col);
			
			}

			Table::Record* pushes[MAXREC];
			int pushescount=0;
			for(i=0;i<(statement->childCount-1)/2-1;i++)
			{
				Table::Record* rec=makeExpression(statement->childList[i*2+2]);
				if(dest==NULL||dest->parameterCount<=i)continue;
				if(rec==NULL)continue;
				if((rec->varType.type==INT&&dest->parametersType[i]==CHR))
				{
					error(SEMANTICSERR,"函数参数类型不正确",statement->childList[i*2+2]->position.line,statement->childList[i*2+2]->position.col);
				}
				pushes[pushescount++]=rec;
				
				//currq->paramType[0]=Quaternion::Record;
			}
			Table::Record* t=addTmpSym();
			create(currq,Quaternion::Q_ALLOC);
			currq->param[0].record=t;
			for(i=0;i<pushescount;i++)
			{
				create(currq,Quaternion::Q_PUSH);
				currq->param[0].record=pushes[i];
			}
			create(currq,Quaternion::Q_CALL_N);
			currq->param[0].contentStr=dest->name;
			
			break;
			
		}
		break;
	case PRINTF:
		if(statement->childCount==7)
		{
			create(currq,Quaternion::Q_PRINTSTR);
			currq->param[0].contentStr=statement->childList[2]->word->word;
			Table::Record* rec=makeExpression(statement->childList[4]);
			if(rec!=NULL&&rec->varType.type==INT){
				create(currq,Quaternion::Q_PRINTINT);
				currq->param[0].record=rec;
			}
			else if(rec!=NULL&&rec->varType.type==CHR)
			{
				create(currq,Quaternion::Q_PRINTCHAR);
				currq->param[0].record=rec;
			}
		}
		else if(statement->childList[2]->isWord)
		{
			create(currq,Quaternion::Q_PRINTSTR);
			currq->param[0].contentStr=statement->childList[2]->word->word;
		}
		else
		{
			Table::Record* rec=makeExpression(statement->childList[2]);
			if(rec!=NULL&&rec->varType.type==INT){
				create(currq,Quaternion::Q_PRINTINT);
				currq->param[0].record=rec;
			}
			else if(rec!=NULL&&rec->varType.type==CHR)
			{
				create(currq,Quaternion::Q_PRINTCHAR);
				currq->param[0].record=rec;
			}
		}
		break;
	case SCANF:
		count=(statement->childCount-1)/2-1;
		for(i=0;i<count;i++)
		{
			if((ident1=getSym(statement->childList[2+i*2]->word->word))==NULL)
			{
				error(SEMANTICSERR,"未声明标识符",statement->childList[i*2+2]->position.line,
					statement->childList[i*2+2]->position.col);
			}
			else if(ident1->identType!=Table::Record::Variable||ident1->varType.isArray)
			{
				error(SEMANTICSERR,"只能对变量赋值",statement->childList[i*2+2]->position.line,
					statement->childList[i*2+2]->position.col);
			}
			create(currq,Quaternion::Q_INPUT);
			currq->param[0].record=ident1;
		}
		break;
	case RETURN:
		if(statement->childCount!=2)
		{
			if(funcRetType==VOID)
			{
				error(SEMANTICSERR,"该函数无返回值",statement->childList[1]->position.line,
					statement->childList[1]->position.col);
			
			}
			Table::Record* rec=makeExpression(statement->childList[2]);
			if(rec==NULL)break;
			if(rec->varType.type==INT&&funcRetType==CHR)
			{
				error(SEMANTICSERR,"无法转换整型到字符型",statement->childList[2]->position.line,
					statement->childList[2]->position.col);
			
			}
			create(currq,Quaternion::Q_RET);
			currq->param[0].record=rec;
		}
		else
		{
			if(funcRetType!=VOID)
			{
				error(SEMANTICSERR,"该函数有返回值",statement->childList[1]->position.line,
					statement->childList[1]->position.col);
			}
			create(currq,Quaternion::Q_RET_N);
		}
		break;
	case SEMICOLON:
		return;
	default:
		break;
	}
}
static void makeComplex(Node* func)
{
	assert(func->nodeType==Complex);
	makeConsts(func->childList[1]);
	makeVaribles(func->childList[2]);

	for(int i=3;i<func->childCount-1;i++)
	{
		makeStatement(func->childList[i]);
	}
}
static void makeFunction(Node* func)
{
	assert(func->nodeType==Function);
	Table::Record* funcsym;
	Quaternion* procname;
	if(hasSym(func->childList[1]->word->word))
	{
		error(GRAMMARERR,"函数名已存在",func->childList[1]->position.line,func->childList[1]->position.col);
		goto para;
	}
	funcRetType=func->childList[0]->word->type;

	funcsym=addSym();
	funcsym->name=func->childList[1]->word->word;
	funcsym->identType=Table::Record::Procedure;
	funcsym->varType.type=func->childList[0]->word->type;
	funcsym->varType.isArray=false;
	incLayer();
	
	procname=addQuaternion();
	procname->command=Quaternion::Q_ASSIGNPROC;
	procname->param[0].record=funcsym;
	int c;
para:
	c=3;
	
	if(func->childList[c]->word->type!=RPAREN){
		while(true)
		{
			Table::Record* rec=addSym();
			rec->identType=Table::Record::Variable;
			rec->name=func->childList[c+1]->word->word;
			rec->varType.type=func->childList[c]->word->type;
			rec->varType.isArray=false;
			
			funcsym->parameters[funcsym->parameterCount]=rec;
			funcsym->parametersType[funcsym->parameterCount]=rec->varType.type;
			funcsym->parameterCount++;
			c+=3;
			if(func->childList[c-1]->word->type==RPAREN)break;
		}
	}
	else
		c++;
	makeComplex(func->childList[c]);
	Quaternion* ret=addQuaternion();
	ret->command=Quaternion::Q_RET_N;
	decLayer();
}
static void makeMain(Node* main)
{
	assert(main->nodeType==Main);
	main->nodeType=Function;
	makeFunction(main);
}
Quaternion* getQuaternion(Node* program,Quaternion* dest)
{
	base=ptr=dest;
	assert(program->nodeType==Program);
	initTable();
	makeConsts(program->childList[0],true);
	makeVaribles(program->childList[1],true);
	Quaternion* endg=addQuaternion();
	endg->command=Quaternion::Q_ENDGLOBAL;
	for(int i=0;i<program->childList[2]->childCount;i++)
		makeFunction(program->childList[2]->childList[i]);
	makeMain(program->childList[3]);
	ptr->command=Quaternion::Q_TERMINATE;
	return base;
}
Quaternion* getQuaternion(Node* program)
{
	Quaternion qs[MAXQ];
	base=ptr=qs;
	assert(program->nodeType==Program);
	initTable();
	makeConsts(program->childList[0],true);
	makeVaribles(program->childList[1],true);
	Quaternion* endg=addQuaternion();
	endg->command=Quaternion::Q_ENDGLOBAL;
	for(int i=0;i<program->childList[2]->childCount;i++)
		makeFunction(program->childList[2]->childList[i]);
	makeMain(program->childList[3]);
	ptr->command=Quaternion::Q_TERMINATE;
	return base;
}