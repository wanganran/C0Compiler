#ifndef ASSEMBLER_H
#define ASSEMBLER_H
#include "compiler.h"
bool getASM(Quaternion* q);
void ASMinit();
void ASMfinalize();
void storeReg(Table::Record* rec);
void loadReg(Table::Record* rec);

void clearReg();
Table::Record** getRegInfo();
#endif