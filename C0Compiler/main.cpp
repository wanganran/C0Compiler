#include <iostream>
#include "compiler.h"
using namespace std;
inline char* Lower(char* str)
{
	for(int i=0;str[i]!=0;i++)
		str[i]=(in(str[i],'A','Z')?(str[i]-'A'+'a'):str[i]);
	return str;
}
void help()
{
	exit(0);
}
int main(int argc,char** argv)
{
	//if(argc<=1)help();
	//if(strcmp(Lower(argv[1]),"help")==0)help();
	char fn[100];
	printf("请输入源代码文件(*.c): ");
	scanf("%s",fn);
	FILE* f=fopen(fn,"r");

	//freopen(fn,"r",stdin);
	//freopen("res.txt","w",stdout);
	char buffer[100000];
	//buffer[100001]=0;
	int tmp;
	int i=0;
	while((tmp=fgetc(f))!=EOF)
		buffer[i++]=(char)tmp;
	fclose(f);
	buffer[i]=0;
	Word* words=new Word[10000];
	
	i=0;
	char* ptr=buffer;
	while(ptr=tryGetWord(ptr,words+i))
		i++;
	words[i].type=ENDOFFILE;

	Node* res=getNodeFrom(Program,words);
	
	Quaternion qs[MAXQ];
	Quaternion optimized[MAXQ];
	Quaternion* nqs=optimized;
	Quaternion* qsptr=qs;
	
	if(getErrorCount()!=0)goto end;
	getQuaternion(res,qs);
	if(getErrorCount()!=0)goto end;
	/*
	while(qsptr=DAGOpt(qsptr,qsptr));
	ShiftDeclares(qs,nqs);
	DataFlowAndASM(optimized);
	*/
	ASMinit();
	qsptr=qs;
	while(qsptr->command!=Quaternion::Q_TERMINATE)
		getASM(qsptr++);
	ASMfinalize();
	
	printf("编译成功！汇编文件名为：%s.asm",FILENAME);
end:
	system("pause");
}