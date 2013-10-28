#ifndef OPTIMIZATION_H
#define OPTIMIZATION_H
#include "compiler.h"

#include <set>
#include <algorithm>
#define MAXT 100
template<typename T>
struct Collection{
	std::set<T> container;
	
	bool Contains(T v)
	{
		return container.find(v)!=container.end();
	}
	void add(T v)
	{
		container.insert(v);
	}
};

struct Block{
	Quaternion* begin;
	Quaternion* end;
	Collection<Block*> inBlock;
	Collection<Block*> outBlock;

	Collection<Table::Record*> in;
	Collection<Table::Record*> out;
	Collection<Table::Record*> use;
	Collection<Table::Record*> def;
};

Quaternion* DAGOpt(Quaternion* src,Quaternion*& dest);
void DataFlowAndASM(Quaternion* start);
void ShiftDeclares(Quaternion* begin,Quaternion* dest);
#endif