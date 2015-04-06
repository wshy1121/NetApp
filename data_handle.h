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
	virtual ~IDealDataHandle() = 0;
	virtual void parseData(TimeCalcInf *pCalcInf);
	static void initDataHandle();
	static RECV_DATA *createRecvData(int contentLen = 0);
	static void destroyRecvData(RECV_DATA *pRecvData);
public: 
	virtual void dealDataHandle (TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf) = 0;
protected:	
	char *m_oper;
	TraceInfoId *m_pTraceInfoId;
	int m_line;
	char *m_fileName;
	char *m_funcName;
	int m_displayLevel;
	const char *m_content;
	int m_contentLen;
	
};


class CCreateCandy : public IDealDataHandle
{
public:
	virtual void dealDataHandle(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf);
};

class CDestroyCandy : public IDealDataHandle
{
public:
	virtual void dealDataHandle(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf);
};

class CInsertTrace : public IDealDataHandle
{
public:
	virtual void dealDataHandle(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf);
};


//---------------------------------
class CDispAll : public IDealDataHandle
{
public:
	virtual void dealDataHandle(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf);
};
class CCleanAll : public IDealDataHandle
{
public:
	virtual void dealDataHandle(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf);
};
class CInsertTag : public IDealDataHandle
{
public:
	virtual void dealDataHandle(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf);
};
class CInsertStrOnly : public IDealDataHandle
{
public:
	virtual void dealDataHandle(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf);
};

class CPrintfMemInfMap : public IDealDataHandle
{
public:
	virtual void dealDataHandle(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf);
};

class CInsertHex : public IDealDataHandle
{
public:
	virtual void dealDataHandle(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf);
};


class COpenFile : public IDealDataHandle
{
public:
	virtual void dealDataHandle(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf);
};

class CCloseFile : public IDealDataHandle
{
public:
	virtual void dealDataHandle(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf);
};

class CVerify : public IDealDataHandle
{
public:
	virtual void dealDataHandle(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf);
};

#endif

