#include "assembler.h"
#include <stdio.h>
int _str_count=0;
static int offset=4;
FILE* fdata,*fins;
#define MAXAC 200
#define ADDASM fprintf
#define ADDASMDB fprintf
char dest[200];
Table::Record* regInfo[5];
Table::Record** getRegInfo()
{
	return regInfo;
}
void ASMinit()
{
	sprintf(dest,"%s.d.tmp",FILENAME);
	fdata=fopen(dest,"w");
	sprintf(dest,"%s.i.tmp",FILENAME);
	fins=fopen(dest,"w");
}
void addASMLine(char* source)
{
	fprintf(fins,"%s\n\n",source);
}
void addASMdb(char* source)
{
	fprintf(fdata,"%s\n\n",source);
}/*
void ADDASMDB(fins,char* str,...) 
{
	va_list arg;
	va_start(arg,str);

	sprintf(dest,str,arg);
	addASMLine(dest);
}
void ADDASMDB(char* str,...) 
{
	va_list arg;
	va_start(arg,str);

	sprintf(dest,str,arg);
	addASMdb(dest);
}
*/
void ASMfinalize()
{
	fclose(fdata);
	fclose(fins);
	
	sprintf(dest,"%s.d.tmp",FILENAME);
	fdata=fopen(dest,"r");
	
	sprintf(dest,"%s.i.tmp",FILENAME);
	fins=fopen(dest,"r");

	
	FILE* fheader0=fopen("fheader0","r");
	FILE* fheader1=fopen("fheader1","r");
	FILE* ffooter=fopen("ffooter","r");

	sprintf(dest,"%s.asm",FILENAME);
	FILE* fout=fopen(dest,"w");
	
	int t;
	while((t=fgetc(fheader0))>=0)
		fputc(t,fout);
	
	while((t=fgetc(fdata))>=0)
		fputc(t,fout);
	
	while((t=fgetc(fheader1))>=0)
		fputc(t,fout);

	while((t=fgetc(fins))>=0)
		fputc(t,fout);

	while((t=fgetc(ffooter))>=0)
		fputc(t,fout);
	
	fclose(fheader0);
	fclose(fheader1);
	fclose(ffooter);
	fclose(fout);
	
	fclose(fins);
	fclose(fdata);
}
void loadReg(Table::Record* rec)
{
	char buff[100];
	char buff2[100];
	Table::Record::Reg back=rec->reg;
	rec->getDest(buff2);
	rec->reg=Table::Record::NOREG;
	rec->getDest(buff);
	rec->reg=back;
	ADDASM(fins,"MOV %s, %s\n",buff2,buff);
}
void storeReg(Table::Record* rec)
{
	char buff[100];
	char buff2[100];
	Table::Record::Reg back=rec->reg;
	rec->getDest(buff2);
	rec->reg=Table::Record::NOREG;
	rec->getDest(buff);
	rec->reg=back;
	ADDASM(fins,"MOV %s, %s\n",buff,buff2);
}
void clearReg()
{
	ADDASM(fins,"XOR EDI, EDI");
	ADDASM(fins,"XOR ESI, ESI");
}
int totalalloc=0;
Table::Record* alloccr[MAXAC];
int alloccv[MAXAC];
int alloccc=0;
bool getASM(Quaternion* q)
{
	int i;
	char buff[100];
	if(!inarr(q->command,2,
		Quaternion::Q_ALLOC,
		Quaternion::Q_ALLOC_C)&&totalalloc>0)
	{
		ADDASMDB(fins,"SUB ESP, %d\n",totalalloc);
		for(i=0;i<alloccc;i++)
		{
			ADDASMDB(fins,"MOV DWORD PTR %s, %d\n",alloccr[i]->getDest(buff),alloccv[i]);

		}
		alloccc=0;
		totalalloc=0;
	}
	switch (q->command)
	{
	case Quaternion::Q_ALLOC:
		q->param[0].record->startAddress=offset;
		offset+=q->param[0].record->getSize();
		totalalloc+=q->param[0].record->getSize();
		//ADDASMDB(fins,"SUB ESP, %d\n",q->param[0].record->getSize());
		break;
	case Quaternion::Q_ALLOC_C:
		q->param[0].record->startAddress=offset;
		offset+=q->param[0].record->getSize();
		totalalloc+=q->param[0].record->getSize();
		//ADDASMDB(fins,"SUB ESP, %d\n",q->param[0].record->getSize());
		alloccr[alloccc]=q->param[0].record;
		if(q->param[0].record->varType.type==INT)
			//ADDASMDB(fins,"MOV DWORD PTR %s, %d\n",q->param[0].record->getDest(buff),q->param[1].contentInt);
			alloccv[alloccc]=q->param[1].contentInt;
		else
			//ADDASMDB(fins,"MOV DWORD PTR %s, %d\n",q->param[0].record->getDest(buff),q->param[1].contentChar);
			alloccv[alloccc]=q->param[1].contentChar;
		alloccc++;
		break;
	case Quaternion::Q_ADD:
		ADDASMDB(fins,"MOV EAX, %s\n",q->param[0].record->getDest(buff));
		ADDASMDB(fins,"ADD EAX, %s\n",q->param[1].record->getDest(buff));
		ADDASMDB(fins,"MOV %s, EAX\n",q->param[2].record->getDest(buff));
		break;
	case Quaternion::Q_ADD_C:
		ADDASMDB(fins,"MOV EAX, %s\n",q->param[0].record->getDest(buff));
		ADDASMDB(fins,"ADD EAX, %d\n",q->param[1].contentInt);
		ADDASMDB(fins,"MOV %s, EAX\n",q->param[2].record->getDest(buff));
		break;
	case Quaternion::Q_SUB:
		ADDASMDB(fins,"MOV EAX, %s\n",q->param[0].record->getDest(buff));
		ADDASMDB(fins,"SUB EAX, %s\n",q->param[1].record->getDest(buff));
		ADDASMDB(fins,"MOV %s, EAX\n",q->param[2].record->getDest(buff));
		break;
	case Quaternion::Q_SUB_C:
		ADDASMDB(fins,"MOV EAX, %s\n",q->param[0].record->getDest(buff));
		ADDASMDB(fins,"SUB EAX, %d\n",q->param[1].contentInt);
		ADDASMDB(fins,"MOV %s, EAX\n",q->param[2].record->getDest(buff));
		break;
	case Quaternion::Q_DIV:
		ADDASMDB(fins,"MOV EAX, %s\n",q->param[0].record->getDest(buff));
		ADDASMDB(fins,"CDQ\n");
		ADDASMDB(fins,"IDIV DWORD PTR %s\n",q->param[1].record->getDest(buff));
		ADDASMDB(fins,"MOV %s, EAX\n",q->param[2].record->getDest(buff));
		break;
	case Quaternion::Q_MUL:
		ADDASMDB(fins,"MOV EAX, %s\n",q->param[0].record->getDest(buff));
		ADDASMDB(fins,"MUL DWORD PTR %s\n",q->param[1].record->getDest(buff));
		ADDASMDB(fins,"MOV %s, EAX\n",q->param[2].record->getDest(buff));
		break;
	case Quaternion::Q_OPP:
		ADDASMDB(fins,"MOV EAX, %s\n",q->param[0].record->getDest(buff));
		ADDASMDB(fins,"NEG EAX\n");
		ADDASMDB(fins,"MOV %s, EAX\n",q->param[2].record->getDest(buff));
		break;
	
	case Quaternion::Q_ET:
		ADDASMDB(fins,"MOV EAX, %s\n",q->param[0].record->getDest(buff));
		ADDASMDB(fins,"CMP EAX, %s\n",q->param[1].record->getDest(buff));
		ADDASMDB(fins,"JNE __%s\n",q->param[2].label->param[0].contentStr);
		break;
	case Quaternion::Q_NET:
		ADDASMDB(fins,"MOV EAX, %s\n",q->param[0].record->getDest(buff));
		ADDASMDB(fins,"CMP EAX, %s\n",q->param[1].record->getDest(buff));
		ADDASMDB(fins,"JE __%s\n",q->param[2].label->param[0].contentStr);
		break;
	case Quaternion::Q_LT:
		ADDASMDB(fins,"MOV EAX, %s\n",q->param[0].record->getDest(buff));
		ADDASMDB(fins,"CMP EAX, %s\n",q->param[1].record->getDest(buff));
		ADDASMDB(fins,"JNL __%s\n",q->param[2].label->param[0].contentStr);
		break;
	case Quaternion::Q_RT:
		ADDASMDB(fins,"MOV EAX, %s\n",q->param[0].record->getDest(buff));
		ADDASMDB(fins,"CMP EAX, %s\n",q->param[1].record->getDest(buff));
		ADDASMDB(fins,"JNG __%s\n",q->param[2].label->param[0].contentStr);
		break;
	case Quaternion::Q_LTET:
		ADDASMDB(fins,"MOV EAX, %s\n",q->param[0].record->getDest(buff));
		ADDASMDB(fins,"CMP EAX, %s\n",q->param[1].record->getDest(buff));
		ADDASMDB(fins,"JG __%s\n",q->param[2].label->param[0].contentStr);
		break;
	case Quaternion::Q_RTET:
		ADDASMDB(fins,"MOV EAX, %s\n",q->param[0].record->getDest(buff));
		ADDASMDB(fins,"CMP EAX, %s\n",q->param[1].record->getDest(buff));
		ADDASMDB(fins,"JL __%s\n",q->param[2].label->param[0].contentStr);
		break;
	case Quaternion::Q_NET_0:
		ADDASMDB(fins,"MOV EAX, %s\n",q->param[0].record->getDest(buff));
		ADDASMDB(fins,"CMP EAX, 0\n");
		ADDASMDB(fins,"JE __%s\n",q->param[2].label->param[0].contentStr);
		break;
	
		
	case Quaternion::Q_ASSIGN:
		ADDASMDB(fins,"MOV EAX, %s\n",q->param[1].record->getDest(buff));
		ADDASMDB(fins,"MOV %s, EAX\n",q->param[0].record->getDest(buff));
		break;
	case Quaternion::Q_ARRR:
		ADDASMDB(fins,"MOV EAX, %s\n",q->param[1].record->getDest(buff));
		ADDASMDB(fins,"SHL EAX, 2\n");
		ADDASMDB(fins,"ADD EAX, %d\n",q->param[0].record->startAddress);
		if(q->param[0].record->isGlobal)
			ADDASMDB(fins,"MOV EAX, [__base+EAX]\n");
		else
			ADDASMDB(fins,"MOV EAX, [EBP+EAX]\n");
		ADDASMDB(fins,"MOV %s, EAX\n",q->param[2].record->getDest(buff));
		break;
	case Quaternion::Q_ARRW:
		ADDASMDB(fins,"MOV EAX, %s\n",q->param[1].record->getDest(buff));
		ADDASMDB(fins,"SHL EAX, 2\n");
		ADDASMDB(fins,"ADD EAX, %d\n",q->param[0].record->startAddress);
		ADDASMDB(fins,"MOV EDX, %s\n",q->param[2].record->getDest(buff));
		if(q->param[0].record->isGlobal)
			ADDASMDB(fins,"MOV [__base+EAX], EDX\n");
		else
			ADDASMDB(fins,"MOV [EBP+EAX], EDX\n");
		break;
		
	
	case Quaternion::Q_GLOBAL:
		ADDASMDB(fdata,"%s byte %d dup(0)\n",q->param[0].record->name,q->param[0].record->getSize());
		q->param[0].record->startAddress=offset;
		offset+=q->param[0].record->getSize();
		break;
	case Quaternion::Q_GLOBAL_C:
		ADDASMDB(fdata,"%s byte %d dup(0)\n",q->param[0].record->name,q->param[0].record->getSize());
		q->param[0].record->startAddress=offset;
		offset+=q->param[0].record->getSize();
		ADDASMDB(fins,"MOV %s, %d\n",q->param[0].record->getDest(buff),q->param[1].contentInt); 
		break;
	case Quaternion::Q_LABEL:
		ADDASMDB(fins,"__%s:\n",q->param[0].contentStr);
		break;
	case Quaternion::Q_ASSIGNPROC:
		ADDASMDB(fins,"@__%s:\n",q->param[0].record->name);
		ADDASMDB(fins,"PUSH EBP\n");
		ADDASMDB(fins,"MOV EBP, ESP\n");
		ADDASMDB(fins,"SUB ESP, 4\n");
		offset=-4;
		for(i=q->param[0].record->parameterCount-1;i>=0;i--)
		{
			offset-=q->param[0].record->parameters[i]->getSize();
			q->param[0].record->parameters[i]->startAddress=offset;
		}
		offset=4;
		break;
	case Quaternion::Q_CALL:
		ADDASMDB(fins,"CALL @__%s\n",q->param[0].contentStr);
		ADDASMDB(fins,"MOV %s, EAX\n",q->param[1].record->getDest(buff));
		break;
	case Quaternion::Q_CALL_N:
		ADDASMDB(fins,"CALL @__%s\n",q->param[0].contentStr);
		break;
	case Quaternion::Q_INPUT:
		if(q->param[0].record->varType.type==INT)
			ADDASMDB(fins,"INVOKE scanf, offset _intin, addr DWORD PTR %s\n",q->param[0].record->getDest(buff));
		else{
			ADDASMDB(fins,"MOV DWORD PTR %s, 0\n",q->param[0].record->getDest(buff));
			ADDASMDB(fins,"INVOKE scanf, offset _charin, addr DWORD PTR %s\n",q->param[0].record->getDest(buff));
		}
		break;
	case Quaternion::Q_JMP:
		ADDASMDB(fins,"JMP __%s\n",q->param[0].label->param[0].contentStr);
		break;
	case Quaternion::Q_PRINTSTR:
		ADDASMDB(fdata,"__STR%d db \"%s\",0\n",_str_count,q->param[0].contentStr);
		ADDASMDB(fins,"INVOKE printf, offset _str, offset __STR%d\n", _str_count++);
		break;
	case Quaternion::Q_PRINTINT:
		ADDASMDB(fins,"INVOKE printf, offset _int, DWORD PTR %s\n",q->param[0].record->getDest(buff));
		break;
	case Quaternion::Q_PRINTCHAR:
		ADDASMDB(fins,"INVOKE printf, offset _char, DWORD PTR %s\n",q->param[0].record->getDest(buff));
		break;
	case Quaternion::Q_PUSH:
		ADDASMDB(fins,"PUSH %s\n",q->param[0].record->getDest(buff));
		break;
	case Quaternion::Q_RET:
		ADDASMDB(fins,"MOV EAX, %s\n",q->param[0].record->getDest(buff));
		ADDASMDB(fins,"MOV ESP, EBP\n");
		ADDASMDB(fins,"POP EBP\n");
		ADDASMDB(fins,"RET\n");
		break;
	case Quaternion::Q_RET_N:
		ADDASMDB(fins,"MOV ESP, EBP\n");
		ADDASMDB(fins,"POP EBP\n");
		ADDASMDB(fins,"RET\n");
		break;
	case Quaternion::Q_ENDGLOBAL:
		ADDASMDB(fins,"CALL @__main\n");
		ADDASMDB(fins,"invoke ExitProcess,0\n");
		break;
	case Quaternion::Q_TERMINATE:
		return false;
	default:
		break;
	}
	return true;
}