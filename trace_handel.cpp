#include "stdafx.h"
#include "time_calc.h"
#include "trace_handel.h"
#include "safe_server.h"
#include "user_manager.h"
#include "log_opr.h"

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
	addMethod("getTraceFileInfs", (IDealDataHandle::Method)&CTraceHandle::getTraceFileInfs);
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
	CLogOprManager::instance()->openFile(m_pTraceInfoId->clientId, (char *)m_content);
}
void CTraceHandle::closeFile(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{	trace_worker();

	parseData(pCalcInf);

	CClientInf *clientInf = pCalcInf->m_clientInf.get();
	clientInf->m_fileName = "";
	CLogOprManager::instance()->closeFile(m_pTraceInfoId->clientId);
}

void CTraceHandle::getTraceFileInfs(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{
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

