#include "stdafx.h"
#include "data_handle.h"
#include "time_calc.h"
#include "safe_server.h"
#include "net_server.h"

using namespace base;
std::map<std::string, IDealDataHandle::MethodInf> IDealDataHandle::m_dealHandleMap;

IDealDataHandle::~IDealDataHandle()
{
}

int IDealDataHandle::addMethod(const char*name, Method method, IDealDataHandle *object)
{
	MethodInf methodInf;
	methodInf.method = method;
	methodInf.object = object;
	
	m_dealHandleMap[name] = methodInf;
	return 0;
}
void IDealDataHandle::execute(TimeCalcInf *pCalcInf)
{
	char *oper = pCalcInf->m_dataInf.m_infs[0];
	if (m_dealHandleMap.find(oper) != m_dealHandleMap.end())
	{
		RECV_DATA *pRecvData = IDealDataHandle::createRecvData();
		TimeCalcInf &calcInf = pRecvData->calcInf;
		CLogDataInf &dataInf = calcInf.m_dataInf;

		dataInf.putInf("OK");
		dataInf.putInf("0");
		dataInf.putInf("0");
		dataInf.putInf("");
		dataInf.putInf("");
		dataInf.putInf("0");
		
		calcInf.m_traceInfoId = pCalcInf->m_traceInfoId;

		MethodInf methodInf = m_dealHandleMap[oper];
		(methodInf.object->*methodInf.method)(pCalcInf, &calcInf);

		char *&packet = calcInf.m_pContent;
		int &packetLen = calcInf.m_contentLen;
		packetLen = dataInf.packet(packet);
		CNetServer::instance()->pushRecvData(pRecvData);
	}
	else
	{
		printf("failed can not find oper  %s\n", oper);
	}
	return ;
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



void IDealDataHandle::initDataHandle()
{
	addMethod("createCandy", (IDealDataHandle::Method)&CCreateCandy::dealDataHandle, new CCreateCandy);
	addMethod("destroyCandy", (IDealDataHandle::Method)&CDestroyCandy::dealDataHandle, new CDestroyCandy);
	addMethod("insertTrace", (IDealDataHandle::Method)&CInsertTrace::dealDataHandle, new CInsertTrace);
	addMethod("dispAll", (IDealDataHandle::Method)&CDispAll::dealDataHandle, new CDispAll);
	addMethod("cleanAll", (IDealDataHandle::Method)&CCleanAll::dealDataHandle, new CCleanAll);
	addMethod("insertTag", (IDealDataHandle::Method)&CInsertTag::dealDataHandle, new CInsertTag);
	addMethod("insertStrOnly", (IDealDataHandle::Method)&CInsertStrOnly::dealDataHandle, new CInsertStrOnly);
	addMethod("printfMemInfMap", (IDealDataHandle::Method)&CPrintfMemInfMap::dealDataHandle, new CPrintfMemInfMap);
	addMethod("insertHex", (IDealDataHandle::Method)&CInsertHex::dealDataHandle, new CInsertHex);
	addMethod("openFile", (IDealDataHandle::Method)&COpenFile::dealDataHandle, new COpenFile);
	addMethod("closeFile", (IDealDataHandle::Method)&CCloseFile::dealDataHandle, new CCloseFile);
	addMethod("verify", (IDealDataHandle::Method)&CVerify::dealDataHandle, new CVerify);
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






