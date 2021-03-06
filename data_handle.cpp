#include "stdafx.h"
#include "data_handle.h"
#include "net_server.h"
#include "user_manager.h"
#include "trace_server.h"

using namespace base;
std::map<std::string, IDealDataHandle::MethodInf> IDealDataHandle::m_dealHandleMap;

IDealDataHandle::~IDealDataHandle()
{
}

int IDealDataHandle::addMethod(const char*name, Method method)
{
	MethodInf methodInf;
	methodInf.method = method;
	methodInf.object = this;
	
	m_dealHandleMap[name] = methodInf;
	return 0;
}
void IDealDataHandle::execute(TimeCalcInf *pCalcInf)
{	trace_worker();  
	char *oper = pCalcInf->m_dataInf->m_infs[0];
    if (oper == NULL)
    {
        return ;
    }
	trace_printf("oper  %s", oper);
	if (m_dealHandleMap.find(oper) != m_dealHandleMap.end())
	{
		RECV_DATA *pRecvData = IDealDataHandle::createRecvData();
		TimeCalcInf &calcInf = pRecvData->calcInf;
		std::shared_ptr<CLogDataInf> &dataInf = calcInf.m_dataInf;

		calcInf.m_traceInfoId = pCalcInf->m_traceInfoId;

		MethodInf methodInf = m_dealHandleMap[oper];
		(methodInf.object->*methodInf.method)(pCalcInf, &calcInf);

		int sessionId = atoi(pCalcInf->m_dataInf->m_infs[1]);
		if (sessionId <= 0)
		{
			IDealDataHandle::destroyRecvData(pRecvData);
			return ;
		}

		calcInf.m_clientInf = pCalcInf->m_clientInf;
		char *&packet = calcInf.m_pContent;
		int &packetLen = calcInf.m_contentLen;
		packetLen = dataInf->getPacket(packet);
		CTraceServer::instance()->pushRecvData(pRecvData);
	}
	else
	{
		printf("failed can not find oper  |%s|\n", oper);
	}
	return ;
}



RECV_DATA *IDealDataHandle::createRecvData(bool isCopy)
{
	RECV_DATA *pRecvData = new RECV_DATA;
	TimeCalcInf *pCalcInf = &pRecvData->calcInf;
	pCalcInf->m_pContent = NULL;
	pCalcInf->m_dataInf = std::shared_ptr<CLogDataInf>(new CLogDataInf(isCopy));
    pCalcInf->m_packet = std::shared_ptr<std::string>(new std::string);
    
	pCalcInf->m_contentLen = 0;

	pCalcInf->m_traceInfoId.threadId = -1;
	pCalcInf->m_traceInfoId.clientId= -1;
	return pRecvData;
}

void IDealDataHandle::destroyRecvData(RECV_DATA *pRecvData)
{
	delete pRecvData;
}






