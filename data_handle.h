#ifndef _DATA_HANDLE_H_
#define _DATA_HANDLE_H_
#include "mem_calc.h"
#include "link_tool.h"


typedef struct TimeCalcInf
{
	TraceInfoId m_traceInfoId;
	base::CLogDataInf m_dataInf;

	char *m_oper;
	int m_line;
	char * m_fileName;
	char * m_funcName;
	int m_displayLevel;
	char *m_pContent;
	int m_contentLen;
}TimeCalcInf;

typedef struct RECV_DATA
{
	TimeCalcInf calcInf;
	struct node node;
}RECV_DATA;	
#define recvDataContain(ptr)  container_of(ptr, RECV_DATA, node)


class IDealDataHandle
{
public: 
	typedef int (IDealDataHandle::*Method)(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf);
	typedef struct 
	{
		Method method;
		IDealDataHandle *object;
	}MethodInf;
	
	virtual ~IDealDataHandle() = 0;
	static RECV_DATA *createRecvData(int contentLen = 0);
	static void destroyRecvData(RECV_DATA *pRecvData);
	int addMethod(const char*name, Method method, IDealDataHandle *object);
	static void execute(TimeCalcInf *pCalcInf);	
protected:	
	static std::map<std::string, MethodInf> m_dealHandleMap;	
};

class CVerify : public IDealDataHandle
{
public:
	virtual void dealDataHandle(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf);
};


#endif

