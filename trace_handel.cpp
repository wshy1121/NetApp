#include "stdafx.h"
#include "time_calc.h"
#include "trace_handel.h"
#include "safe_server.h"
#include "user_manager.h"
#include "log_opr.h"
#include "net_client.h"

using namespace base;
extern CPthreadMutex g_insMutexCalc;

CTraceHandle::CTraceHandle()
{
	addMethod("createCandy", (IDealDataHandle::Method)&CTraceHandle::createCandy);
	addMethod("destroyCandy", (IDealDataHandle::Method)&CTraceHandle::destroyCandy);
	addMethod("insertTrace", (IDealDataHandle::Method)&CTraceHandle::insertTrace);
	addMethod("dispAll", (IDealDataHandle::Method)&CTraceHandle::dispAll);
	addMethod("cleanAll", (IDealDataHandle::Method)&CTraceHandle::cleanAll);
	addMethod("insertTag", (IDealDataHandle::Method)&CTraceHandle::insertTag);
	addMethod("insertStrOnly", (IDealDataHandle::Method)&CTraceHandle::insertStrOnly);
	addMethod("printfMemInfMap", (IDealDataHandle::Method)&CTraceHandle::printfMemInfMap);
	addMethod("insertHex", (IDealDataHandle::Method)&CTraceHandle::insertHex);
	addMethod("openFile", (IDealDataHandle::Method)&CTraceHandle::openFile);
	addMethod("closeFile", (IDealDataHandle::Method)&CTraceHandle::closeFile);
	addMethod("getTraceFileList", (IDealDataHandle::Method)&CTraceHandle::getTraceFileList);
}
void CTraceHandle::parseData(TimeCalcInf *pCalcInf)
{
	base::CLogDataInf &dataInf = pCalcInf->m_dataInf;
	m_oper = dataInf.m_infs[0];

	char *tid = pCalcInf->m_dataInf.m_infs[2];
	m_pTraceInfoId = &(pCalcInf->m_traceInfoId);
	m_pTraceInfoId->threadId = atoi(tid);

	m_line = atoi(dataInf.m_infs[3]);
	m_fileName = dataInf.m_infs[4];;
	m_funcName = dataInf.m_infs[5];
	m_displayLevel = atoi(dataInf.m_infs[6]);
	m_content = dataInf.m_infs[7];
	m_contentLen = dataInf.m_infLens[7] + 1;
}

void CTraceHandle::createCandy(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{	trace_worker();
	
	parseData(pCalcInf);
	CTimeCalc *pTimeCalc = CTimeCalc::createCTimeCalc(m_line, m_fileName, m_funcName, m_displayLevel, *m_pTraceInfoId);
	return ;
}



void CTraceHandle::destroyCandy(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{	trace_worker();

	parseData(pCalcInf);
	FuncTraceInfo_t *TraceInfo = CTimeCalcManager::instance()->GetTraceInf(*m_pTraceInfoId);
	if (TraceInfo == NULL || TraceInfo->pCalcList->size() == 0)
	{
		return ;
	}
	
	CTimeCalc *pTimeCalc = CTimeCalcContain(TraceInfo->pCalcList->back());
	CTimeCalc::destroyCTimeCalc(pTimeCalc);
	return ;
}

void CTraceHandle::insertTrace(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{	trace_worker();

	parseData(pCalcInf);
	CTimeCalcManager::instance()->InsertTrace(m_line, m_fileName, *m_pTraceInfoId, m_content);
}


void CTraceHandle::dispAll(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{	trace_worker();

	parseData(pCalcInf);
	CTimeCalcManager::instance()->DispAll(m_pTraceInfoId->clientId, m_content);
}

void CTraceHandle::cleanAll(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{	trace_worker();

	parseData(pCalcInf);
	CTimeCalcManager::instance()->cleanAll(m_pTraceInfoId->clientId);
}
void CTraceHandle::insertTag(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{	trace_worker();

	parseData(pCalcInf);
	CTimeCalcManager::instance()->InsertTag(*m_pTraceInfoId, m_line, m_fileName, m_content);
}

void CTraceHandle::insertStrOnly(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{	trace_worker();

	parseData(pCalcInf);
	CTimeCalcManager::instance()->InsertStrOnly(*m_pTraceInfoId, m_content);
}


void CTraceHandle::printfMemInfMap(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{	trace_worker();

	parseData(pCalcInf);
	CTimeCalcManager::instance()->printfMemInfMap(*m_pTraceInfoId);
}

void CTraceHandle::insertHex(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{	trace_worker();

	parseData(pCalcInf);
	CTimeCalcManager::instance()->InsertHex(*m_pTraceInfoId, m_line, m_fileName, (char *)m_content, m_contentLen);
}


void CTraceHandle::openFile(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{	trace_worker();

	parseData(pCalcInf);

	CClientInf *clientInf = pCalcInf->m_clientInf.get();
	clientInf->m_fileName = m_content;
	clientInf->m_traceFileInf = CLogOprManager::instance()->openFile(m_pTraceInfoId->clientId, (char *)m_content);
}
void CTraceHandle::closeFile(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{	trace_worker();

	parseData(pCalcInf);

	CClientInf *clientInf = pCalcInf->m_clientInf.get();
	clientInf->m_fileName = "";
	CLogOprManager::instance()->closeFile(m_pTraceInfoId->clientId);
}

void CTraceHandle::getTraceFileList(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{	trace_worker();

	base::CLogDataInf &reqDataInf = pCalcInf->m_dataInf;
	char *oper = reqDataInf.m_infs[0];
	char *sessionId = reqDataInf.m_infs[1];

	CClientInf *clientInf = pCalcInf->m_clientInf.get();
	base::CLogDataInf &repDataInf = repCalcInf->m_dataInf;
	repDataInf.putInf(oper);
	repDataInf.putInf(sessionId);//session id(大于0)

	TraceFileInf *traceFileInf = NULL;
	CLogOprManager::TraceFileInfMap &traceFileMap = CLogOprManager::instance()->getTraceFileList();
	CLogOprManager::TraceFileInfMap::iterator iter = traceFileMap.begin();
	for (; iter != traceFileMap.end(); ++iter)
	{
		traceFileInf = iter->second;
		repDataInf.putInf((char *)traceFileInf->m_fileName.c_str());
	}

	repDataInf.packet();
	return ;
}



CTraceClient *CTraceClient::_instance;

CTraceClient *CTraceClient::instance()
{
	if (NULL == _instance)
	{
		CGuardMutex guardMutex(g_insMutexCalc);
		if (NULL == _instance)
		{
			_instance = new CTraceClient;
		}
	}
	return _instance;
}
CTraceClient::CTraceClient()
{
}

bool CTraceClient::getTraceFileList(StrVec &fileList)
{	trace_worker();
	char sessionId[16];
	snprintf(sessionId, sizeof(sessionId), "%d", CNetClient::instance()->getSessionId());

	CLogDataInf dataInf;
	dataInf.putInf((char *)"getTraceFileList");
	dataInf.putInf(sessionId);//session id(大于0)

	char *packet = NULL;
	int packetLen = dataInf.packet(packet);
	CNetClient::instance()->send(packet, packetLen);
	CNetClient::instance()->receiveInfData(&dataInf);

	if (dataInf.m_infsNum == 0)
	{	trace_printf("NULL");
		return false;
	}

	for (int i=2; i<dataInf.m_infsNum; ++i)
	{
		fileList.push_back(dataInf.m_infs[i]);
		trace_printf("dataInf.m_infs[i]  %s", dataInf.m_infs[i]);
	}
	return true;
}




