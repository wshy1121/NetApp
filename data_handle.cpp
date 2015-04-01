#include "stdafx.h"
#include "data_handle.h"
#include "time_calc.h"

IDealDataHandle::~IDealDataHandle()
{
}

void IDealDataHandle::parseData(TimeCalcInf *pCalcInf)
{
	m_opr = pCalcInf->m_opr;
	m_pTraceInfoId = &(pCalcInf->m_traceInfoId);
	m_line = pCalcInf->m_line;
	m_fileName = pCalcInf->m_fileName;
	m_funcName = pCalcInf->m_funcName;
	m_displayLevel = pCalcInf->m_displayLevel;
	m_content = pCalcInf->m_pContent;
	m_contentLen = pCalcInf->m_contentLen;
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
}


void CCreateCandy::dealDataHandle (TimeCalcInf *pCalcInf)
{
	parseData(pCalcInf);
	CTimeCalc *pTimeCalc = CTimeCalc::createCTimeCalc(m_line, m_fileName, m_funcName, m_displayLevel, *m_pTraceInfoId);
	return ;
}



void CDestroyCandy::dealDataHandle (TimeCalcInf *pCalcInf)
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

void CInsertTrace::dealDataHandle (TimeCalcInf *pCalcInf)
{
	parseData(pCalcInf);
	CTimeCalcManager::instance()->InsertTrace(m_line, m_fileName, *m_pTraceInfoId, m_content);
}


void CDispAll::dealDataHandle (TimeCalcInf *pCalcInf)
{
	parseData(pCalcInf);
	CTimeCalcManager::instance()->DispAll(m_pTraceInfoId->clientId, m_content);
}

void CCleanAll::dealDataHandle (TimeCalcInf *pCalcInf)
{
	parseData(pCalcInf);
	CTimeCalcManager::instance()->cleanAll(m_pTraceInfoId->clientId);
}
void CInsertTag::dealDataHandle (TimeCalcInf *pCalcInf)
{
	parseData(pCalcInf);
	CTimeCalcManager::instance()->InsertTag(*m_pTraceInfoId, m_line, m_fileName, m_content);
}

void CInsertStrOnly::dealDataHandle (TimeCalcInf *pCalcInf)
{
	parseData(pCalcInf);
	CTimeCalcManager::instance()->InsertStrOnly(*m_pTraceInfoId, m_content);
}


void CPrintfMemInfMap::dealDataHandle (TimeCalcInf *pCalcInf)
{
	parseData(pCalcInf);
	CTimeCalcManager::instance()->printfMemInfMap(*m_pTraceInfoId);
}

void CInsertHex::dealDataHandle (TimeCalcInf *pCalcInf)
{
	parseData(pCalcInf);
	CTimeCalcManager::instance()->InsertHex(*m_pTraceInfoId, m_line, m_fileName, (char *)m_content, m_contentLen);
}


void COpenFile::dealDataHandle (TimeCalcInf *pCalcInf)
{
	parseData(pCalcInf);
	CTimeCalcManager::instance()->openFile(*m_pTraceInfoId, (char *)m_content);
}
void CCloseFile::dealDataHandle (TimeCalcInf *pCalcInf)
{
	parseData(pCalcInf);
	CTimeCalcManager::instance()->closeFile(*m_pTraceInfoId);
}








