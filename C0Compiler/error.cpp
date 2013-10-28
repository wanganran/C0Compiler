#include "error.h"
#include <stdio.h>
int errcount=0;
void error(int type,char* msg,int line,int col)
{
	switch (type)
	{
	case WORDERR:
		printf("�ʷ�����:");
		break;
	case GRAMMARERR:
		printf("�﷨����:");
		break;
	case SEMANTICSERR:
		printf("�������:");
		break;
	case OUTERR:
		printf("�������ɴ���:");
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