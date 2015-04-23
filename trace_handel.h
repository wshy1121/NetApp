#ifndef _TRACE_HANDLE_H_
#define _TRACE_HANDLE_H_
#include "data_handle.h"

class CTraceHandle : public IDealDataHandle
{
public:
	CTraceHandle();
public:
	void createCandy(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf);
	void destroyCandy(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf);
	void insertTrace(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf);
	void dispAll(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf);
	void cleanAll(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf);
	void insertTag(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf);
	void insertStrOnly(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf);
	void printfMemInfMap(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf);
	void insertHex(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf);
	void openFile(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf);
	void closeFile(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf);
	void getTraceFileInfs(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf);
private:	
	void parseData(TimeCalcInf *pCalcInf);
private:
	char *m_oper;
	TraceInfoId *m_pTraceInfoId;
	int m_line;
	char *m_fileName;
	char *m_funcName;
	int m_displayLevel;
	const char *m_content;
	int m_contentLen;
};
#endif

