#include "error.h"
#include <stdio.h>
int errcount=0;
void error(int type,char* msg,int line,int col)
{
	switch (type)
	{
	case WORDERR:
		printf("词法错误:");
		break;
	case GRAMMARERR:
		printf("语法错误:");
		break;
	case SEMANTICSERR:
		printf("语义错误:");
		break;
	case OUTERR:
		printf("代码生成错误:");
		break;
	default:
		break;
	}
	printf("%s\n\tat line %d col %d\n",msg,line,col);
	errcount++;
}
int getErrorCount()
{
	return errcount;
}