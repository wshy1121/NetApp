#include "stdafx.h"
#include "data_handle.h"
#include "time_calc.h"

IDealDataHandle::~IDealDataHandle()
{
}

void IDealDataHandle::parseData(TimeCalcInf *pCalcInf)
{
	base::CLogDataInf &dataInf = pCalcInf->m_dataInf;
	m_oper = dataInf.m_infs[0];
	m_pTraceInfoId = &(pCalcInf->m_traceInfoId);
	m_line = atoi(dataInf.m_infs[2]);
	m_fileName = dataInf.m_infs[3];;
	m_funcName = dataInf.m_infs[4];
	m_displayLevel = atoi(dataInf.m_infs[5]);
	m_content = dataInf.m_infs[6];
	m_contentLen = dataInf.m_infLens[6] + 1;
}

void IDealDataHandle::initDataHandle()
{
	CTimeCalcInfManager *_instance = CTimeCalcInfManager::instance();
	_instance->registerHandle("createCandy", new CCreateCandy);
	_instance->registerHandle("destroyCandy", new CDestroyCandy);
	_instance->registerHandle("insertTrace", new CInsertTrace);
	_instance->registerHandle("dispAll", new CDispAll);
	_instance->registerHandle("cleanAll", new CCleanAll);
	_instance->registerHandle("insertTag", new CInsertTag);
	_instance->registerHandle("insertStrOnly", new CInsertStrOnly);
	_instance->registerHandle("printfMemInfMap", new CPrintfMemInfMap);
	_instance->registerHandle("insertHex", new CInsertHex);
	_instance->registerHandle("openFile", new COpenFile);
	_instance->registerHandle("closeFile", new CCloseFile);
	_instance->registerHandle("verify", new CVerify);
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


void CCreateCandy::dealDataHandle(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{
	parseData(pCalcInf);
	CTimeCalc *pTimeCalc = CTimeCalc::createCTimeCalc(m_line, m_fileName, m_funcName, m_displayLevel, *m_pTraceInfoId);
	return ;
}



void CDestroyCandy::dealDataHandle(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{
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

void CInsertTrace::dealDataHandle(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{
	parseData(pCalcInf);
	CTimeCalcManager::instance()->InsertTrace(m_line, m_fileName, *m_pTraceInfoId, m_content);
}


void CDispAll::dealDataHandle(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{
	parseData(pCalcInf);
	CTimeCalcManager::instance()->DispAll(m_pTraceInfoId->clientId, m_content);
}

void CCleanAll::dealDataHandle(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{
	parseData(pCalcInf);
	CTimeCalcManager::instance()->cleanAll(m_pTraceInfoId->clientId);
}
void CInsertTag::dealDataHandle(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{
	parseData(pCalcInf);
	CTimeCalcManager::instance()->InsertTag(*m_pTraceInfoId, m_line, m_fileName, m_content);
}

void CInsertStrOnly::dealDataHandle(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{
	parseData(pCalcInf);
	CTimeCalcManager::instance()->InsertStrOnly(*m_pTraceInfoId, m_content);
}


void CPrintfMemInfMap::dealDataHandle(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{
	parseData(pCalcInf);
	CTimeCalcManager::instance()->printfMemInfMap(*m_pTraceInfoId);
}

void CInsertHex::dealDataHandle(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{
	parseData(pCalcInf);
	CTimeCalcManager::instance()->InsertHex(*m_pTraceInfoId, m_line, m_fileName, (char *)m_content, m_contentLen);
}


void COpenFile::dealDataHandle(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{
	parseData(pCalcInf);
	CTimeCalcManager::instance()->openFile(*m_pTraceInfoId, (char *)m_content);
}
void CCloseFile::dealDataHandle(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{
	parseData(pCalcInf);
	CTimeCalcManager::instance()->closeFile(*m_pTraceInfoId);
}


void CVerify::dealDataHandle(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{
	printf("CVerify::dealDataHandle \n");
}






