
#include "optimization.h"
#include <map>
#include <stack>

using namespace std;

#define NODEMAX 1000

struct DAGNodeItem{
	//bool isImmediate;
	//union{
	//	int imme;
	Table::Record* rec;
	//} value;
	int layer;
	DAGNodeItem()
	{
		layer=0;
	}
	bool operator <(const DAGNodeItem& it) const
	{
		return rec<it.rec;
	}
};

typedef set<DAGNodeItem> DAGNode;
DAGNode nodelist[NODEMAX];
int nodecount=0;
inline DAGNode* NewNode()
{
	nodelist[nodecount].clear();
	return nodelist+(nodecount++);
}
map<Table::Record*,DAGNode*> itemMaxLayer;

//find the node of max layer x
inline DAGNode* findMaxNode(Table::Record* x)
{
	return itemMaxLayer[x];
}
//find the node item of max layer x in node
//x must in node.
inline DAGNodeItem findMaxNodeItem(DAGNode* node,Table::Record* x)
{
	DAGNodeItem res;
	for(DAGNode::iterator it=node->begin();it!=node->end();it++)
	{
		if(it->rec==x)
		{
			if(res.layer<it->layer)
			{
				res=*it;			
			}
		}
	}
	return res;
}

inline DAGNode* findMaxNodeOrCreate(Table::Record* x)
{
	DAGNode* res=findMaxNode(x);
	if(res==0)
	{
		DAGNodeItem item;
		item.rec=x;
		item.layer=0;
		DAGNode* nd=NewNode();
		nd->insert(item);
		res=nd;
		itemMaxLayer[x]=res;
	}
	return res;
}

//return -1 if rec not exists
int findMaxLayer(Table::Record* rec)
{
	/*for(int i=nodecount-1;i>=0;i--)
	{
		DAGNode& node=nodelist[i];
		for(DAGNode::iterator it=node.begin();it!=node.end();it++)
			if(it->rec==rec)
				return it->layer;
	}*/
	DAGNode* node=itemMaxLayer[rec];
	if(node!=0)
	{
		return findMaxNodeItem(node,rec).layer;
	}
	else 
		return -1;
}
inline DAGNode* insertMaxNodeItem(Table::Record* rec, DAGNode* parent)
{
	int prev=findMaxLayer(rec);
	DAGNodeItem item;
	item.rec=rec;
	item.layer=prev+1;
	itemMaxLayer[rec]=parent;
	parent->insert(item);
	return parent;
}

//dest is the destination to rewrite,
//src is the source pointer to read at(Block start).
//return value is where the next DAGOpt need to read at(Block end).
Quaternion* DAGOpt(Quaternion* src,Quaternion*& dest)
{
#define ABSORB if(q!=dest)*dest=*q;dest++;
	//2-op 2-rec operation
	map<DAGNode*,map<DAGNode*,map<Quaternion::Command,DAGNode*> > > DAG2;
	//2-op 1-rec 1-imme operation
	map<DAGNode*,map<int,map<Quaternion::Command,DAGNode*> > > DAGImme;
	//ARRW operation
	map<DAGNode*,map<DAGNode*,map<DAGNode*,DAGNode*> > > DAGARRW;
	//1-op operation
	map<DAGNode*,map<Quaternion::Command,DAGNode*> > DAG1;
	
	//reset all
	nodecount=0;
	itemMaxLayer.clear();

	//start DAG;
	for(Quaternion* q=src;;q++)
	{
		if(inarr(q->command,10,
			Quaternion::Q_ET,
			Quaternion::Q_LT,
			Quaternion::Q_LTET,
			Quaternion::Q_RTET,
			Quaternion::Q_RT,
			Quaternion::Q_NET,
			Quaternion::Q_NET_0,
			Quaternion::Q_JMP,
			Quaternion::Q_RET,
			Quaternion::Q_RET_N,
			Quaternion::Q_LABEL
			))
		{
			//absorb it and return
			ABSORB;
			return q+1;
		}
		else if(inarr(q->command,1,
			Quaternion::Q_ASSIGNPROC))
		{
			ABSORB;
			return q+1;
		}
		else if(inarr(q->command,1,
			Quaternion::Q_TERMINATE))
		{
			ABSORB;
			//program over,return 0
			return 0;
		}
		else if(inarr(q->command,5,
			Quaternion::Q_ADD,
			Quaternion::Q_SUB,
			Quaternion::Q_MUL,
			Quaternion::Q_DIV,
			Quaternion::Q_ARRR))
		{
			//2-op 2-rec calc
			DAGNode* r1=findMaxNodeOrCreate(q->param[0].record);
			DAGNode* r2=findMaxNodeOrCreate(q->param[1].record);
			
			DAGNode* parent=DAG2[r1][r2][q->command];
			if(!parent)
			{
				//parent doesn't exist, create one
				DAGNodeItem item;
				item.rec=q->param[2].record;
				int layer=findMaxLayer(q->param[2].record);
				item.layer=layer+1;
				
				parent=NewNode();
				DAG2[r1][r2][q->command]=parent;
				
				parent->insert(item);
				itemMaxLayer[q->param[2].record]=parent;
				ABSORB;
			}
			else
			{
				//parent exists. has a top layer value?
				bool finished=false;
				for(DAGNode::iterator it=parent->begin();
					it!=parent->end();
					it++)
				{
					if(it->layer==findMaxLayer(it->rec))
					{
						//have a top layer value, assign to it.
						//find the previous node that has max layer result
						//DAGNode* prev=findMaxNode(q->param[2].record);
						insertMaxNodeItem(q->param[2].record,parent);

						dest->command=Quaternion::Q_ASSIGN;
						dest->param[0]=q->param[2];
						dest->param[1].record=it->rec;
						dest++;
						finished=true;
						break;
					}
				}
				if(!finished)
				{
					//doesn't have a top layer value. absorb it.
					insertMaxNodeItem(q->param[2].record,parent);

					ABSORB;
				}
			}
		}
		else if(inarr(q->command,2,
			Quaternion::Q_ADD_C,
			Quaternion::Q_SUB_C))
		{
			//parem[1] is a constance
			DAGNode* r1=findMaxNodeOrCreate(q->param[0].record);
			
			DAGNode* parent=DAGImme[r1][q->param[1].contentInt][q->command];
			if(!parent)
			{
				//parent doesn't exist, create one
				DAGNodeItem item;
				item.rec=q->param[2].record;
				int layer=findMaxLayer(q->param[2].record);
				item.layer=layer+1;
				
				parent=NewNode();
				DAGImme[r1][q->param[1].contentInt][q->command]=parent;
				
				parent->insert(item);
				itemMaxLayer[q->param[2].record]=parent;
				ABSORB;
			}
			else
			{
				//parent exists. has a top layer value?
				bool finished=false;
				for(DAGNode::iterator it=parent->begin();
					it!=parent->end();
					it++)
				{
					if(it->layer==findMaxLayer(it->rec))
					{
						//have a top layer value, assign to it.
						//find the previous node that has max layer result
						//DAGNode* prev=findMaxNode(q->param[2].record);
						insertMaxNodeItem(q->param[2].record,parent);

						dest->command=Quaternion::Q_ASSIGN;
						dest->param[0]=q->param[2];
						dest->param[1].record=it->rec;
						dest++;
						finished=true;
						break;
					}
				}
				if(!finished)
				{
					//doesn't have a top layer value. absorb it.
					insertMaxNodeItem(q->param[2].record,parent);

					ABSORB;
				}
			}
		}
		else if(inarr(q->command,1,
			Quaternion::Q_OPP))
		{
			//parem[1] is null
			DAGNode* r1=findMaxNodeOrCreate(q->param[0].record);
			
			DAGNode* parent=DAG1[r1][q->command];
			if(!parent)
			{
				//parent doesn't exist, create one
				DAGNodeItem item;
				item.rec=q->param[2].record;
				int layer=findMaxLayer(q->param[2].record);
				item.layer=layer+1;
				
				parent=NewNode();
				DAG1[r1][q->command]=parent;
				
				parent->insert(item);
				itemMaxLayer[q->param[2].record]=parent;
				ABSORB;
			}
			else
			{
				//parent exists. has a top layer value?
				bool finished=false;
				for(DAGNode::iterator it=parent->begin();
					it!=parent->end();
					it++)
				{
					if(it->layer==findMaxLayer(it->rec))
					{
						//have a top layer value, assign to it.
						//find the previous node that has max layer result
						//DAGNode* prev=findMaxNode(q->param[2].record);
						insertMaxNodeItem(q->param[2].record,parent);

						dest->command=Quaternion::Q_ASSIGN;
						dest->param[0]=q->param[2];
						dest->param[1].record=it->rec;
						dest++;
						finished=true;
						break;
					}
				}
				if(!finished)
				{
					//doesn't have a top layer value. absorb it.
					insertMaxNodeItem(q->param[2].record,parent);

					ABSORB;
				}
			}
		}
		else if(inarr(q->command,1,
			Quaternion::Q_ARRW))
		{
			//renew the array
			DAGNode* newarr=NewNode();
			insertMaxNodeItem(q->param[0].record,newarr);
			
			ABSORB;
		}
		else if(inarr(q->command,1,
			Quaternion::Q_ASSIGN))
		{
			DAGNode* nd=findMaxNodeOrCreate(q->param[1].record);
			insertMaxNodeItem(q->param[0].record,nd);

			ABSORB;
		}
		else if(inarr(q->command,7,
			Quaternion::Q_ALLOC,
			Quaternion::Q_ALLOC_C,
			Quaternion::Q_PUSH,
			Quaternion::Q_CALL_N,
			Quaternion::Q_PRINTCHAR,
			Quaternion::Q_PRINTINT,
			Quaternion::Q_PRINTSTR))
		{
			//no need to optimize
			ABSORB;
		}
		else if(inarr(q->command,1,
			Quaternion::Q_CALL))
		{
			DAGNode* nd=NewNode();
			insertMaxNodeItem(q->param[1].record,nd);

			ABSORB;
		}
		else if(inarr(q->command,1,
			Quaternion::Q_INPUT))
		{
			DAGNode* nd=NewNode();
			insertMaxNodeItem(q->param[0].record,nd);

			ABSORB;
		}
		else{
			ABSORB;
		}
	}

	
}
void ShiftDeclares(Quaternion* begin,Quaternion* dest)
{
	int state=0;
	while(begin->command!=Quaternion::Q_ASSIGNPROC)
	{
		*dest=*begin;
		begin++;
		dest++;
	}
	*dest=*begin;
	dest++;begin++;
	
	while(1){
		Quaternion* back=begin;
		while(!inarr(begin->command,2,Quaternion::Q_ASSIGNPROC,Quaternion::Q_TERMINATE))
		{
			if(inarr(begin->command,2,Quaternion::Q_ALLOC,Quaternion::Q_ALLOC_C))
			{
				*dest=*begin;
				dest++;
			}
			begin++;
		}
		begin=back;
		while(!inarr(begin->command,2,Quaternion::Q_ASSIGNPROC,Quaternion::Q_TERMINATE))
		{
			if(!inarr(begin->command,2,Quaternion::Q_ALLOC,Quaternion::Q_ALLOC_C))
			{
				*dest=*begin;
				dest++;
			}
			begin++;
		}
		*dest=*begin;
		dest++;
		if(begin->command==Quaternion::Q_TERMINATE)
			break;
		begin++;
	}
}
Table::Record* findMaxRecord(Block* b,bool& use)
{
	set<Table::Record*>::iterator it;
	for(it=b->def.container.begin();
		it!=b->def.container.end();
		it++)
	{
		if(!((*it)->isGlobal)&&(*it)->reg==Table::Record::NOREG)
		{
			use=false;
			return *it;
		}
	}
	for(it=b->use.container.begin();
		it!=b->use.container.end();
		it++)
	{
		if(!((*it)->isGlobal)&&(*it)->reg==Table::Record::NOREG)
		{
			use=true;
			return *it;
		}
	}
	return 0;
}
void DataFlowAndASM(Quaternion* begin)
{
#define MAXB 100
	ASMinit();
	for(Quaternion* q=begin;;)
	{
		while(q->command!=Quaternion::Q_ASSIGNPROC&&
			q->command!=Quaternion::Q_TERMINATE)
		{
			getASM(q++);
		}
		if(q->command==Quaternion::Q_TERMINATE)
		{
			getASM(q);
			ASMfinalize();
			return;
		}
		else if(q->command==Quaternion::Q_ASSIGNPROC)
		{
			set<Table::Record*> allocnotc;
			getASM(q++);
			while(inarr(q->command,2,Quaternion::Q_ALLOC,
				Quaternion::Q_ALLOC_C)){
					if(q->command==Quaternion::Q_ALLOC)
						allocnotc.insert(q->param[0].record);
					getASM(q++);
			}
			Block blocks[MAXB];
			std::map<char*,int> labelBlock;
			int blockcount=0;
			//i is where the procedure ends at last
			int i;
			for(i=0;!inarr(q[i].command,2,
				Quaternion::Q_ASSIGNPROC,
				Quaternion::Q_TERMINATE);i++)
			{
				blocks[blockcount].begin=q+i;
				if(q[i].command==Quaternion::Q_LABEL)
				{
					labelBlock[q[i].param[0].contentStr]=blockcount;
					i++;
				}
				else if(inarr(q[i].command,6,
					Quaternion::Q_CALL,
					Quaternion::Q_CALL_N,
					Quaternion::Q_INPUT,
					Quaternion::Q_PRINTCHAR,
					Quaternion::Q_PRINTINT,
					Quaternion::Q_PRINTSTR))
				{
					blocks[blockcount].end=q+i;
					blockcount++;
					continue;
				}
				while(!inarr(q[i].command,17,
					Quaternion::Q_ET,
					Quaternion::Q_LT,
					Quaternion::Q_LTET,
					Quaternion::Q_RTET,
					Quaternion::Q_RT,
					Quaternion::Q_NET,
					Quaternion::Q_NET_0,
					Quaternion::Q_JMP,
					Quaternion::Q_RET,
					Quaternion::Q_RET_N,
					Quaternion::Q_LABEL,
					Quaternion::Q_CALL,
					Quaternion::Q_CALL_N,
					Quaternion::Q_INPUT,
					Quaternion::Q_PRINTCHAR,
					Quaternion::Q_PRINTINT,
					Quaternion::Q_PRINTSTR
					))
				{
					i++;
				}
				if(inarr(q[i].command,7,
						Quaternion::Q_CALL,
						Quaternion::Q_CALL_N,
						Quaternion::Q_INPUT,
						Quaternion::Q_PRINTCHAR,
						Quaternion::Q_PRINTINT,
						Quaternion::Q_PRINTSTR,
						Quaternion::Q_LABEL))
						blocks[blockcount].end=q+(--i);
				else
					blocks[blockcount].end=q+i;
				blockcount++;
			}
			q+=i;
			Block* returnblock=blocks+blockcount;
			for(i=0;i<blockcount;i++)
			{
#define addDef(q) if(!blocks[i].use.Contains(q))blocks[i].def.add(q);
#define addUse(q) if(!blocks[i].def.Contains(q))blocks[i].use.add(q);
				for(Quaternion* qi=blocks[i].begin;qi<=blocks[i].end;qi++)
				{
					if(inarr(qi->command,10,
						Quaternion::Q_ET,
						Quaternion::Q_LT,
						Quaternion::Q_LTET,
						Quaternion::Q_RTET,
						Quaternion::Q_RT,
						Quaternion::Q_NET
						
						))
					{
						addUse(qi->param[0].record);
						addUse(qi->param[1].record);
					}
					else if(inarr(qi->command,1,
						Quaternion::Q_NET_0))
					{
						addUse(qi->param[0].record);
					}
					else if(inarr(qi->command,5,
						Quaternion::Q_ADD,
						Quaternion::Q_SUB,
						Quaternion::Q_MUL,
						Quaternion::Q_DIV))
					{
						//2-op 2-rec calc
						addUse(qi->param[0].record);
						addUse(qi->param[1].record);
						addDef(qi->param[2].record);
					}
					
					else if(inarr(qi->command,3,
						Quaternion::Q_ADD_C,
						Quaternion::Q_SUB_C,
						Quaternion::Q_OPP))
					{
						addUse(qi->param[0].record);
						addDef(qi->param[2].record);
					}
					else if(qi->command==Quaternion::Q_ARRR)
					{
						addUse(qi->param[1].record);
						addDef(qi->param[2].record);
					}
					else if(qi->command==Quaternion::Q_ARRW)
					{
						addUse(qi->param[1].record);
						addUse(qi->param[2].record);
					}
					else if(inarr(qi->command,1,
						Quaternion::Q_ASSIGN))
					{
						addUse(qi->param[1].record);
						addDef(qi->param[0].record);
					}
					else if(inarr(qi->command,1,
						Quaternion::Q_PUSH
						))
					{
						addUse(qi->param[0].record);
					}
					else if(inarr(qi->command,1,
						Quaternion::Q_CALL
						))
					{
						addDef(qi->param[1].record);
					}
					else if(inarr(qi->command,1,
						Quaternion::Q_INPUT))
					{
						addDef(qi->param[0].record);
					}
					else if(inarr(qi->command,2,
						Quaternion::Q_PRINTCHAR,
						Quaternion::Q_PRINTINT))
					{
						addUse(qi->param[0].record);
					}
					else if(inarr(qi->command,1,
						Quaternion::Q_RET))
					{
						addUse(qi->param[0].record);
					}
				}
				if(inarr(blocks[i].end->command,7,
					Quaternion::Q_ET,
					Quaternion::Q_LT,
					Quaternion::Q_LTET,
					Quaternion::Q_RTET,
					Quaternion::Q_RT,
					Quaternion::Q_NET,
					Quaternion::Q_NET_0
					))
				{
					Block* to=blocks+labelBlock[blocks[i].end->param[2].label->param[0].contentStr];
					blocks[i].outBlock.add(to);
					to->inBlock.add(blocks+i);
					blocks[i].outBlock.add(blocks+i+1);
				}
				else if(inarr(blocks[i].end->command,1,
					Quaternion::Q_JMP))
				{
					Block* to=blocks+labelBlock[blocks[i].end->param[0].label->param[0].contentStr];

					blocks[i].outBlock.add(to);
					to->inBlock.add(blocks+i);
				}
				else if(inarr(blocks[i].end->command,2,
					Quaternion::Q_RET,
					Quaternion::Q_RET_N
					))
				{
					blocks[i].outBlock.add(returnblock);
					returnblock->inBlock.add(blocks+i);

				}
				else 
				{
					blocks[i].outBlock.add(blocks+i+1);
					blocks[i+1].inBlock.add(blocks+i);
				}

			}
			for(int c=0;c<10;c++){
				for(i=blockcount-1;i>=0;i--)
				{
					set<Block*>::iterator it;
					for(it=blocks[i].outBlock.container.begin();
						it!=blocks[i].outBlock.container.end();
						it++)
					{
						for(set<Table::Record*>::iterator it2=(*it)->in.container.begin();
							it2!=(*it)->in.container.end();
							it2++)
							blocks[i].out.container.insert(*it2);
					}
					set<Table::Record*> minus;
					set<Table::Record*>::iterator it2;
					for(it2=blocks[i].out.container.begin();
						it2!=blocks[i].out.container.end();
						it2++)
					{
						if(blocks[i].def.container.find(*it2)==blocks[i].def.container.end())
						{
							minus.insert(*it2);
						}
					}
					blocks[i].in=blocks[i].use;
					for(it2=minus.begin();
						it2!=minus.end();
						it2++)
						blocks[i].in.container.insert(*it2);
				}
			}
			if(blockcount>0)
			{
				for(set<Table::Record*>::iterator tt=blocks[0].use.container.begin();
					tt!=blocks[0].use.container.end();
					tt++)
				{
					if(allocnotc.find(*tt)!=allocnotc.end())
					{
						char errstr[100];
						sprintf(errstr,"警告：未赋值便使用变量%s",(*tt)->name);
						error(OUTERR,errstr,0,0);
					}
				}
			}
			//only EBX,ECX,ESI,EDI can store global variable
			Table::Record** reginfo=getRegInfo();
			reginfo[0]=reginfo[1]=reginfo[2]=reginfo[3]=0;
			for(i=0;i<blockcount;i++)
			{
				if(inarr(blocks[i].begin->command,6,
					Quaternion::Q_CALL,
					Quaternion::Q_CALL_N,
					Quaternion::Q_INPUT,
					Quaternion::Q_PRINTCHAR,
					Quaternion::Q_PRINTINT,
					Quaternion::Q_PRINTSTR))
				{
					if(inarr(blocks[i].begin->command,2,
						Quaternion::Q_PRINTCHAR,
						Quaternion::Q_PRINTINT))
					{
						Table::Record* rec=(*blocks[i].use.container.begin());
						if(rec->reg!=0)
							loadReg(rec);
					}
					getASM(blocks[i].begin);
					if(inarr(blocks[i].begin->command,2,
						Quaternion::Q_CALL,
						Quaternion::Q_INPUT))
					{

						Table::Record* rec=(*blocks[i].def.container.begin());
						if(rec->reg!=0)
							storeReg(rec);
					}
				}
				else{
					
					Quaternion* currq=blocks[i].begin;
					int j;
					if(currq->command==Quaternion::Q_LABEL && blocks[i].begin==blocks[i].end)
					{
						getASM(currq);
						goto finish;
					}
					if(currq->command==Quaternion::Q_LABEL)
						getASM(currq++);
					
					for(j=0;j<4;j++)
					{
						if(reginfo[j]==0)
						{
							bool use;
							Table::Record* mr=findMaxRecord(blocks+i,use);
							if(mr!=0){
								reginfo[j]=mr;
								mr->reg=(Table::Record::Reg)(j+1);
								if(use)loadReg(mr);
							}
						}
						else
						{
							if(blocks[i].in.Contains(reginfo[j]))
								loadReg(reginfo[j]);
						}
					}
					
					
					for(;currq<blocks[i].end;currq++)
						getASM(currq);

					if(inarr(currq->command,10,
						Quaternion::Q_ET,
						Quaternion::Q_LT,
						Quaternion::Q_LTET,
						Quaternion::Q_RTET,
						Quaternion::Q_RT,
						Quaternion::Q_NET,
						Quaternion::Q_NET_0,
						Quaternion::Q_JMP,
						Quaternion::Q_RET,
						Quaternion::Q_RET_N))
					{
						
						for(j=0;j<4;j++)
						{
							if(reginfo[j]!=0)
							{
								if(blocks[i].out.Contains(reginfo[j]))
								{
									storeReg(reginfo[j]);
								}
							}
						}
						getASM(currq);
						for(j=0;j<4;j++)
							if(reginfo[j]!=0)
								if(!blocks[i].out.Contains(reginfo[j]))
								{
									reginfo[j]->reg=Table::Record::NOREG;
									reginfo[j]=0;
								}
					}
					else{
						getASM(currq);
						for(j=0;j<4;j++)
						{
							if(reginfo[j]!=0)
							{
								if(blocks[i].out.Contains(reginfo[j]))
								{
									storeReg(reginfo[j]);
								}
								else
								{
									reginfo[j]->reg=Table::Record::NOREG;
									reginfo[j]=0;
								}
							}
						}
					}
finish:
					continue;
				}
			}
			//clearReg();
			/*map<Table::Record*,int> reccount;
			for(i=0;i<blockcount;i++)
			{
				int mul=blocks[i].inBlock.container.size()*3;
				for(set<Table::Record*>::iterator it=blocks[i].use.container.begin();
					it!=blocks[i].use.container.end();
					it++)
				{
					reccount[*it]+=mul+1;
				}
				for(set<Table::Record*>::iterator it=blocks[i].def.container.begin();
					it!=blocks[i].def.container.end();
					it++)
				{
					reccount[*it]+=mul+1;
				}
			}
			int maxc[4]={0};
			Table::Record* maxr[4];
			for(map<Table::Record*,int>::iterator it=reccount.begin();
				it!=reccount.end();
				it++)
			{
				for(i=0;i<4;i++)
					if(it->second>maxc[i])
					{
						for(int j=3;j>i;j--)
						{
							maxc[j]=maxc[j-1];
							maxr[j]=maxr[j-1];
						}
						maxc[i]=it->second;
						maxr[i]=it->first;
						break;
					}
			}
			reginfo[4]=reginfo[1]=reginfo[2]=reginfo[3]=0;
			for(i=0;maxr[i]!=0&&i<4;i++){
				reginfo[i+1]=maxr[i];
				maxr[i]->reg=(Table::Record::Reg)(i+1);
				if(blocks[0].in.container.find(maxr[i])!=blocks[0].in.container.end())
				{
					loadReg(maxr[i]);
				}
			}
			for(i=0;i<blockcount;i++)
			{
				
				if(inarr(blocks[i].begin->command,6,
					Quaternion::Q_CALL,
					Quaternion::Q_CALL_N,
					Quaternion::Q_INPUT,
					Quaternion::Q_PRINTCHAR,
					Quaternion::Q_PRINTINT,
					Quaternion::Q_PRINTSTR
					))
				{
					for(int i=0;i<4;i++)
					{
						if(maxr[i]->isGlobal)
						{
							storeReg(maxr[i]);
						}
						else if(blocks[i].out.Contains(maxr[i]))
						{
							storeReg(maxr[i]);
						}
					}
				}
				for(Quaternion* qq=blocks[i].begin;qq<blocks[i].end;qq++)
				{
					getASM(qq);
				}
				if(inarr(blocks[i].end->command,2,
					Quaternion::Q_RET,
					Quaternion::Q_RET_N))
				{
					for(int i=0;i<4;i++)
					{
						if(maxr[i]->isGlobal)
						{
							storeReg(maxr[i]);
						}
					}
					getASM(blocks[i].end);
				}
			}*/

		}

	}
	//ASMfinalize();
}