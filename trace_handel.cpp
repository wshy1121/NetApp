#include "stdafx.h"
#include "time_calc.h"
#include "trace_handel.h"
#include "safe_server.h"

CTraceHandle::CTraceHandle()
{
	addMethod("createCandy", (IDealDataHandle::Method)&CTraceHandle::createCandy, this);
	addMethod("destroyCandy", (IDealDataHandle::Method)&CTraceHandle::destroyCandy, this);
	addMethod("insertTrace", (IDealDataHandle::Method)&CTraceHandle::insertTrace, this);
	addMethod("dispAll", (IDealDataHandle::Method)&CTraceHandle::dispAll, this);
	addMethod("cleanAll", (IDealDataHandle::Method)&CTraceHandle::cleanAll, this);
	addMethod("insertTag", (IDealDataHandle::Method)&CTraceHandle::insertTag, this);
	addMethod("insertStrOnly", (IDealDataHandle::Method)&CTraceHandle::insertStrOnly, this);
	addMethod("printfMemInfMap", (IDealDataHandle::Method)&CTraceHandle::printfMemInfMap, this);
	addMethod("insertHex", (IDealDataHandle::Method)&CTraceHandle::insertHex, this);
	addMethod("openFile", (IDealDataHandle::Method)&CTraceHandle::openFile, this);
	addMethod("closeFile", (IDealDataHandle::Method)&CTraceHandle::closeFile, this);
	addMethod("verify", (IDealDataHandle::Method)&CTraceHandle::verify, this);
}
void CTraceHandle::parseData(TimeCalcInf *pCalcInf)
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


void CTraceHandle::createCandy(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{
	parseData(pCalcInf);
	CTimeCalc *pTimeCalc = CTimeCalc::createCTimeCalc(m_line, m_fileName, m_funcName, m_displayLevel, *m_pTraceInfoId);
	return ;
}



void CTraceHandle::destroyCandy(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
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

void CTraceHandle::insertTrace(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{
	parseData(pCalcInf);
	CTimeCalcManager::instance()->InsertTrace(m_line, m_fileName, *m_pTraceInfoId, m_content);
}


void CTraceHandle::dispAll(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{
	parseData(pCalcInf);
	CTimeCalcManager::instance()->DispAll(m_pTraceInfoId->clientId, m_content);
}

void CTraceHandle::cleanAll(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{
	parseData(pCalcInf);
	CTimeCalcManager::instance()->cleanAll(m_pTraceInfoId->clientId);
}
void CTraceHandle::insertTag(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{
	parseData(pCalcInf);
	CTimeCalcManager::instance()->InsertTag(*m_pTraceInfoId, m_line, m_fileName, m_content);
}

void CTraceHandle::insertStrOnly(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{
	parseData(pCalcInf);
	CTimeCalcManager::instance()->InsertStrOnly(*m_pTraceInfoId, m_content);
}


void CTraceHandle::printfMemInfMap(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{
	parseData(pCalcInf);
	CTimeCalcManager::instance()->printfMemInfMap(*m_pTraceInfoId);
}

void CTraceHandle::insertHex(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{
	parseData(pCalcInf);
	CTimeCalcManager::instance()->InsertHex(*m_pTraceInfoId, m_line, m_fileName, (char *)m_content, m_contentLen);
}


void CTraceHandle::openFile(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{
	parseData(pCalcInf);
	CTimeCalcManager::instance()->openFile(*m_pTraceInfoId, (char *)m_content);
}
void CTraceHandle::closeFile(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{
	parseData(pCalcInf);
	CTimeCalcManager::instance()->closeFile(*m_pTraceInfoId);
}



void CTraceHandle::verify(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{
	base::CLogDataInf &dataInf = pCalcInf->m_dataInf;

	char *oper = dataInf.m_infs[0];
	char *sessionId = dataInf.m_infs[1];
	
	char *keyInf = dataInf.m_infs[2];
	int keyInfLen = dataInf.m_infLens[2];
	
	char *userName = dataInf.m_infs[3];	
	int userNameLen = dataInf.m_infLens[3];
	
	char *passWord = dataInf.m_infs[4];
	int passWordLen = dataInf.m_infLens[4];


	CSafeServer::instance()->decode(keyInf, keyInfLen, userName, userNameLen,userName);
	CSafeServer::instance()->decode(keyInf, keyInfLen, passWord, passWordLen,passWord);
	printf("CVerify::dealDataHandle %s  %s  %s\n", oper, userName, passWord);

	{
		base::CLogDataInf &dataInf = repCalcInf->m_dataInf;
		
		char keyInf[KEY_INF_LEN];
		CSafeServer::instance()->createKeyInf(keyInf, sizeof(keyInf));

		char _userName[32];
		int _userNameLen = sizeof(_userName);
		CSafeServer::instance()->encode(keyInf, sizeof(keyInf), userName, strlen(userName)+1, _userName, _userNameLen);

		char _passWord[32];
		int _passWordLen = sizeof(_passWord);
		CSafeServer::instance()->encode(keyInf, sizeof(keyInf), passWord, strlen(passWord)+1, _passWord, _passWordLen);

		dataInf.putInf(oper);
		dataInf.putInf(sessionId);//session id(大于0)
		dataInf.putInf(keyInf, sizeof(keyInf));//密钥
		dataInf.putInf(_userName, _userNameLen);//用户名
		dataInf.putInf(_passWord, _passWordLen); //密码

	}
}




