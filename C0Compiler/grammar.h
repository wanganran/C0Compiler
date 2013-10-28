#ifndef GRAMMAR_H
#define GRAMMAR_H
#include "compiler.h"

Node* getNodeFrom(NodeType type,Word*& words);
inline Node* dolike(int type,Word*& words)
{
	if(type<=ENDOFFILE)
		if(type==words->type){
			return new Node(words++);
		}
		else return NULL;
	else
		return getNodeFrom((NodeType)type,words);
}
inline Node* like(NodeType type, Word*& arr,int count,...)
{
	Word* back=arr;
	Node* nd=new Node(type,0);
	va_list ap;
	va_start(ap,count);
	for(int i=0;i<count;i++){
		Node* res=dolike(va_arg(ap,int),arr);
		if(!res){
			arr=back;
			return NULL;
		}
		else
		{
			nd->addChild(res);
		}
	}
	va_end(ap);
	return nd;
}
inline void find(Word*& words,WordType type){
	if(words->type==ENDOFFILE){
		error(GRAMMARERR,"预料之外的文件结束",words->line,words->col);
		return;
	}
	if(words->type==type)return;
	find(++words,type);
}

inline void find(Word*& words,WordType type1,WordType type2){
	if(words->type==ENDOFFILE){
		error(GRAMMARERR,"预料之外的文件结束",words->line,words->col);
		return;
	}
	if(words->type==type1||words->type==type2)return;
	find(++words,type1,type2);
}
inline void find(Word*& words,WordType type1,WordType type2,WordType type3){
	if(words->type==ENDOFFILE){
		error(GRAMMARERR,"预料之外的文件结束",words->line,words->col);
		return;
	}
	if(words->type==type1||words->type==type2||words->type==type3)return;
	find(++words,type1,type2,type3);
}
#endif