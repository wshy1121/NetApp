#ifndef _DATA_HANDLE_H_
#define _DATA_HANDLE_H_
#include <memory>
#include "mem_calc.h"
#include "link_tool.h"
#include "user_manager.h"
#include "trace_packet.h"

typedef struct TimeCalcInf
{
	TraceInfoId m_traceInfoId;
	std::shared_ptr<CLogDataInf> m_dataInf;
	std::shared_ptr<std::string> m_packet;
	std::shared_ptr<IClientInf> m_clientInf;
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
	static RECV_DATA *createRecvData(int contentLen = 0, bool isCopy = true);
	static void destroyRecvData(RECV_DATA *pRecvData);
	int addMethod(const char*name, Method method);
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

