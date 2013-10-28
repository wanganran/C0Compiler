const int WORDERR=1;
const int GRAMMARERR=2;
const int SEMANTICSERR=3;
const int OUTERR=4;
void error(int type,char* msg,int line,int col);
int getErrorCount();