#include "compiler.h"
Table::Record* addSym();
Table::Record* addTmpSym();
Table::Record* clearTmpSym();
bool incLayer();
bool decLayer();
void initTable();
Table::Record* getSym(char* name);
Table::Record* hasSym(char* name);