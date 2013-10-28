#ifndef COMPILER_H
#define COMPILER_H
const int MAXWL=20;
#include "stdarg.h"
#define FILENAME "output"

#define in(i,a,b) ((i)>=(a)&&(i)<=(b))
inline bool inarr(char i,char* arr,int len)
{
	while(len--)
	{
		if(arr[len]==i)return true;
	}
	return false;
}
template<typename T>
inline bool inarr(T val,int count,...)
{
	va_list ap;
	va_start(ap,count);
	for(int i=0;i<count;i++)
		if(val==va_arg(ap,T))
			return true;
	va_end(ap);
	return false;

}

#include "structures.h"
#include "error.h"
#include "word.h"
#include "grammar.h"
#include "semantics.h"
#include "optimization.h"
#include "assembler.h"

#endif