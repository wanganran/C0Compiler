#ifndef SEMANTICS_H
#define SEMANTICS_H
#include "compiler.h"
#define MAXQ 5000

Quaternion* getQuaternion(Node* program);
Quaternion* getQuaternion(Node* Program,Quaternion* dest);
#endif