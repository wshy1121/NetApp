#include "stdafx.h"
#include "data_handle.h"
#include "net_server.h"
#include "user_manager.h"

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
	char *oper = pCalcInf->m_dataInf.m_infs[0];		
	if (m_dealHandleMap.find(oper) != m_dealHandleMap.end())
	{
		RECV_DATA *pRecvData = IDealDataHandle::createRecvData();
		TimeCalcInf &calcInf = pRecvData->calcInf;
		CLogDataInf &dataInf = calcInf.m_dataInf;

		calcInf.m_traceInfoId = pCalcInf->m_traceInfoId;

		MethodInf methodInf = m_dealHandleMap[oper];
		(methodInf.object->*methodInf.method)(pCalcInf, &calcInf);

		int sessionId = atoi(pCalcInf->m_dataInf.m_infs[1]);
		if (sessionId <= 0)
		{
			IDealDataHandle::destroyRecvData(pRecvData);
			return ;
		}
		char *&packet = calcInf.m_pContent;
		int &packetLen = calcInf.m_contentLen;
		packetLen = dataInf.getPacket(packet);
		CNetServer::instance()->pushRecvData(pRecvData);
	}
	else
	{
		printf("failed can not find oper  %s\n", oper);
	}
	return ;
}



RECV_DATA *IDealDataHandle::createRecvData(int contentLen)
{
	RECV_DATA *pRecvData = new RECV_DATA;
	TimeCalcInf *pCalcInf = &pRecvData->calcInf;
	pCalcInf->m_contentLen = contentLen;
	
	pCalcInf->m_oper = NULL;
	pCalcInf->m_traceInfoId.threadId = -1;
	pCalcInf->m_traceInfoId.clientId= -1;
	pCalcInf->m_line = -1;
	pCalcInf->m_fileName = NULL;
	pCalcInf->m_funcName = NULL;
	pCalcInf->m_displayLevel = -1;
	return pRecvData;
}

void IDealDataHandle::destroyRecvData(RECV_DATA *pRecvData)
{
	delete pRecvData;
}






