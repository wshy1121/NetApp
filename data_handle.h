#ifndef _DATA_HANDLE_H_
#define _DATA_HANDLE_H_
#include "mem_calc.h"


typedef struct TimeCalcInf
{
	char *m_oper;
	TraceInfoId m_traceInfoId;
	int m_line;
	char * m_fileName;
	char * m_funcName;
	int m_displayLevel;
	char *m_pContent;
	int m_contentLen;

	char *m_memBuffer;
	char *infs[INF_SIZE];
	int infLens[INF_SIZE];
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
public: 
	virtual void dealDataHandle (TimeCalcInf *pCalcInf) = 0;
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
	virtual void dealDataHandle (TimeCalcInf *pCalcInf);
};

class CDestroyCandy : public IDealDataHandle
{
public:
	virtual void dealDataHandle (TimeCalcInf *pCalcInf);
};

class CInsertTrace : public IDealDataHandle
{
public:
	virtual void dealDataHandle (TimeCalcInf *pCalcInf);
};


//---------------------------------
class CDispAll : public IDealDataHandle
{
public:
	virtual void dealDataHandle (TimeCalcInf *pCalcInf);
};
class CCleanAll : public IDealDataHandle
{
public:
	virtual void dealDataHandle (TimeCalcInf *pCalcInf);
};
class CInsertTag : public IDealDataHandle
{
public:
	virtual void dealDataHandle (TimeCalcInf *pCalcInf);
};
class CInsertStrOnly : public IDealDataHandle
{
public:
	virtual void dealDataHandle (TimeCalcInf *pCalcInf);
};

class CPrintfMemInfMap : public IDealDataHandle
{
public:
	virtual void dealDataHandle (TimeCalcInf *pCalcInf);
};

class CInsertHex : public IDealDataHandle
{
public:
	virtual void dealDataHandle (TimeCalcInf *pCalcInf);
};


class COpenFile : public IDealDataHandle
{
public:
	virtual void dealDataHandle (TimeCalcInf *pCalcInf);
};

class CCloseFile : public IDealDataHandle
{
public:
	virtual void dealDataHandle (TimeCalcInf *pCalcInf);
};

#endif

