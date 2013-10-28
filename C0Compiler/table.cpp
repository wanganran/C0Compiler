#include "table.h"
#include "string.h"
Table identTable;
int _tmp_count=0;
Table::Record* addSym(){
	return identTable.recs[identTable.recsCount++]=new Table::Record();
}
Table::Record* addTmpSym()
{
	Table::Record* rec=(identTable.recs[identTable.recsCount++]=new Table::Record());
	rec->identType=Table::Record::Variable;
	rec->varType.isArray=false;
	rec->varType.type=INT;
	char* dest=new char[10];
	sprintf(dest,"__tmp%d",_tmp_count++);
	rec->name=dest;
	return rec;
}
bool incLayer()
{
	identTable.Layers[++identTable.currLayer]=identTable.recsCount;
	return true;
}
bool decLayer()
{
	identTable.recsCount=identTable.Layers[identTable.currLayer--];
	return true;
}
void initTable()
{
	identTable.currLayer=0;
	identTable.recsCount=0;
}
Table::Record* getSym(char* name)
{
	for(int i=identTable.recsCount-1;i>=0;i--)
	{
		if(strcmp(name,identTable.recs[i]->name)==0)return identTable.recs[i];
	}
	return NULL;
}
Table::Record* hasSym(char* name)
{
	for(int i=identTable.recsCount-1;i>=identTable.Layers[identTable.currLayer];i--)
	{
		if(strcmp(name,identTable.recs[i]->name)==0)return identTable.recs[i];
	}
	return NULL;
}