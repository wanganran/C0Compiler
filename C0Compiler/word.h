#ifndef WORD_H
#define WORD_H
#include "compiler.h"

WordType search(char* str);
char* subString(char* begin,char* end);
char* tryGetWord(char* str,Word* res);
void outputDemo(char* content);
#endif