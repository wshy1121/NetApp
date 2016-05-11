#include "stdafx.h"
#include "time_calc.h"
#include "trace_handel.h"
#include "safe_server.h"
#include "user_manager.h"
#include "log_opr.h"
#include "net_client.h"
#include "string_base.h"

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
	addMethod("cleanFile", (IDealDataHandle::Method)&CTraceHandle::cleanFile);
	addMethod("getTraceFileList", (IDealDataHandle::Method)&CTraceHandle::getTraceFileList);
	addMethod("getTraceFileInf", (IDealDataHandle::Method)&CTraceHandle::getTraceFileInf);
}
void CTraceHandle::parseData(TimeCalcInf *pCalcInf)
{
	std::shared_ptr<CLogDataInf> &dataInf = pCalcInf->m_dataInf;
	m_oper = dataInf->m_infs[0];

	char *tid = pCalcInf->m_dataInf->m_infs[2];
	m_pTraceInfoId = &(pCalcInf->m_traceInfoId);
	m_pTraceInfoId->threadId = atol(tid);

	m_line = atoi(dataInf->m_infs[3]);
	m_fileName = dataInf->m_infs[4];;
	m_funcName = dataInf->m_infs[5];
	m_displayLevel = atoi(dataInf->m_infs[6]);
	m_content = dataInf->m_infs[7];
	m_contentLen = dataInf->m_infLens[7] + 1;

	m_preLine = 0;
	m_preFileName = (char *)"";
	m_preFuncName = (char *)"";
	if (8 < dataInf->m_infsNum) m_preLine = atoi(dataInf->m_infs[8]);
	if (9 < dataInf->m_infsNum) m_preFileName = dataInf->m_infs[9];
	if (10 < dataInf->m_infsNum) m_preFuncName = dataInf->m_infs[10];
}

void CTraceHandle::createCandy(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{	trace_worker();
	TraceFileInf *traceFileInf = pCalcInf->m_clientInf.get()->m_traceFileInf;
    if (traceFileInf == NULL)
    {
        return ;
    }
    
	parseData(pCalcInf);
	
	traceFileInf->m_candyCount++;
	traceFileInf->m_lastCandy = m_funcName;

	int line[2] = {m_line, m_preLine};
	char *fileName[2] = {m_fileName, m_preFileName};
	char *funcName[2] = {m_funcName, m_preFuncName};
	
	CTimeCalc::createCTimeCalc(line, fileName, funcName, m_displayLevel, *m_pTraceInfoId);
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
	if (strcmp(pTimeCalc->m_FuncName, m_funcName) || strcmp(pTimeCalc->m_FileName, m_fileName))
	{
		CTimeCalcManager::instance()->InsertTrace(m_line, m_fileName, *m_pTraceInfoId, "lost trace_exit");
		return ;
	}
	CTimeCalc::destroyCTimeCalc(pTimeCalc);
	return ;
}

void CTraceHandle::insertTrace(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{	trace_worker();
	TraceFileInf *traceFileInf = pCalcInf->m_clientInf.get()->m_traceFileInf;
    if (traceFileInf == NULL)
    {
        return ;
    }

	parseData(pCalcInf);
	traceFileInf->m_traceCount++;
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

	IClientInf *clientInf = pCalcInf->m_clientInf.get();
	clientInf->m_fileName = m_content;
    std::string clientIpAddr = inet_ntoa(clientInf->m_clientAddr.sin_addr);
	clientInf->m_traceFileInf = CLogOprManager::instance()->openFile(m_pTraceInfoId->clientId, (char *)m_content, clientIpAddr);
    printf("clientIpAddr.c_str()  %s\n", clientIpAddr.c_str());
}
void CTraceHandle::closeFile(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{	trace_worker();

	parseData(pCalcInf);

	IClientInf *clientInf = pCalcInf->m_clientInf.get();
	clientInf->m_fileName = "";
	CLogOprManager::instance()->closeFile(m_pTraceInfoId->clientId);
    clientInf->m_traceFileInf = NULL;
}

void CTraceHandle::cleanFile(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{	trace_worker();
	std::shared_ptr<CLogDataInf> &dataInf = pCalcInf->m_dataInf;
	trace_printf("NULL");
	char *fileName = dataInf->m_infs[2];
	trace_printf("fileName  %s", fileName);
    
	CLogOprManager::instance()->cleanFile(m_pTraceInfoId->clientId);
	trace_printf("NULL");	
	return ;
}

void CTraceHandle::getTraceFileList(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{	trace_worker();

	std::shared_ptr<CLogDataInf> &reqDataInf = pCalcInf->m_dataInf;
	char *oper = reqDataInf->m_infs[0];
	char *sessionId = reqDataInf->m_infs[1];

	//IClientInf *clientInf = pCalcInf->m_clientInf.get();z
	std::shared_ptr<CLogDataInf> &repDataInf = repCalcInf->m_dataInf;
	repDataInf->putInf(oper);
	repDataInf->putInf(sessionId);//session id(大于0)

	char fileSizeMem[1024];
	char *fileSize = fileSizeMem;
	int fileMemIndex = 0;
	int strCount = 0;
	TraceFileInf *traceFileInf = NULL;
	CLogOprManager::TraceFileInfMap traceFileMap;
    CLogOprManager::instance()->getTraceFileList(traceFileMap);
	CLogOprManager::TraceFileInfMap::iterator iter = traceFileMap.begin();
	for (; iter != traceFileMap.end(); ++iter)
	{
		traceFileInf = iter->second;
		strCount = base::snprintf(fileSize, sizeof(fileSizeMem)-fileMemIndex, "%d", traceFileInf->m_fileSize) + 1;
		repDataInf->putInf((char *)traceFileInf->m_fileName.c_str());
		repDataInf->putInf(fileSize);
		fileSize += strCount;
		fileMemIndex += strCount;
		
		trace_printf("traceFileInf->m_fileName.c_str(), traceFileInf->m_fileSize  %s  %s", traceFileInf->m_fileName.c_str(), fileSize);
	}

	repDataInf->packet();
	return ;
}

void CTraceHandle::getTraceFileInf(TimeCalcInf *pCalcInf, TimeCalcInf *repCalcInf)
{	trace_worker();

	std::shared_ptr<CLogDataInf> &reqDataInf = pCalcInf->m_dataInf;
	char *oper = reqDataInf->m_infs[0];
	char *sessionId = reqDataInf->m_infs[1];
	char *fileName = reqDataInf->m_infs[2];

	CLogOprManager::TraceFileInfMap traceFileMap;
    CLogOprManager::instance()->getTraceFileList(traceFileMap);
	CLogOprManager::TraceFileInfMap::iterator iter = traceFileMap.find(fileName);
	if (iter == traceFileMap.end())
	{	trace_printf("NULL");
		return ;
	}
	
	std::shared_ptr<CLogDataInf> &repDataInf = repCalcInf->m_dataInf;
	repDataInf->putInf(oper);
	repDataInf->putInf(sessionId);//session id(大于0)

	TraceFileInf *traceFileInf = iter->second;
	char fileSize[32];
	char fileCount[32];
	char candyCount[32];
	char traceCount[32];
	
	base::snprintf(fileSize, sizeof(fileSize), "%d", traceFileInf->m_fileSize);
	base::snprintf(fileCount, sizeof(fileCount), "%d", traceFileInf->m_count);
	base::snprintf(candyCount, sizeof(fileCount), "%d", traceFileInf->m_candyCount);
	base::snprintf(traceCount, sizeof(fileCount), "%d", traceFileInf->m_traceCount);
	repDataInf->putInf((char *)traceFileInf->m_fileName.c_str());
	repDataInf->putInf(fileSize);
	repDataInf->putInf(fileCount);
	repDataInf->putInf(candyCount);
	repDataInf->putInf(traceCount);
	repDataInf->putInf((char *)traceFileInf->m_lastCandy.c_str());
	
	repDataInf->packet();
	trace_printf("getTraceFileInf  %s  %d  %d", traceFileInf->m_fileName.c_str(), traceFileInf->m_fileSize, traceFileInf->m_count);
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

bool CTraceClient::cleanFile(const char *fileName)
{	trace_worker();
	trace_printf("fileName  %s", fileName);
	char sessionId[16];
	base::snprintf(sessionId, sizeof(sessionId), "%d", CNetClient::instance()->getSessionId());

	CLogDataInf dataInf;
	dataInf.putInf((char *)"cleanFile");
	dataInf.putInf(sessionId);//session id(大于0)
	dataInf.putInf((char *)fileName);

	char *packet = NULL;
	int packetLen = dataInf.packet(packet);
	CNetClient::instance()->dealPacket(packet, packetLen, dataInf);

	if (dataInf.m_infsNum == 0)
	{	trace_printf("NULL");
		return false;
	}	
	return true;
}

bool CTraceClient::getTraceFileList(TraceFileVec &fileList)
{	trace_worker();
	char sessionId[16];
	base::snprintf(sessionId, sizeof(sessionId), "%d", CNetClient::instance()->getSessionId());

	CLogDataInf dataInf;
	dataInf.putInf((char *)"getTraceFileList");
	dataInf.putInf(sessionId);//session id(大于0)

	char *packet = NULL;
	int packetLen = dataInf.packet(packet);
	CNetClient::instance()->dealPacket(packet, packetLen, dataInf);
	
	if (dataInf.m_infsNum == 0)
	{	trace_printf("NULL");
		return false;
	}

	TraceFileInf traceFileInf;
	for (int i=2; i<dataInf.m_infsNum; i+=2)
	{
		traceFileInf.m_fileName = dataInf.m_infs[i];
		traceFileInf.m_fileSize= atoi(dataInf.m_infs[i+1]);
		trace_printf("traceFileInf.m_fileName, traceFileInf.m_fileSize  %s  %d", traceFileInf.m_fileName.c_str(), traceFileInf.m_fileSize);
		fileList.push_back(traceFileInf);
	}
	return true;
}


bool CTraceClient::getTraceFileInf(const char *fileName, TraceFileInf &traceFileInf)
{	trace_worker();
	trace_printf("fileName  %s", fileName);
	char sessionId[16];
	base::snprintf(sessionId, sizeof(sessionId), "%d", CNetClient::instance()->getSessionId());
	CLogDataInf dataInf;
	dataInf.putInf((char *)"getTraceFileInf");
	dataInf.putInf(sessionId);//session id(大于0)
	dataInf.putInf((char *)fileName);

	char *packet = NULL;
	int packetLen = dataInf.packet(packet);
	CNetClient::instance()->dealPacket(packet, packetLen, dataInf);
	

	if (dataInf.m_infsNum == 0)
	{	trace_printf("NULL");
		return false;
	}

	traceFileInf.m_fileName = dataInf.m_infs[2];
	traceFileInf.m_fileSize = atoi(dataInf.m_infs[3]);
	traceFileInf.m_count = atoi(dataInf.m_infs[4]);
	traceFileInf.m_candyCount = atoi(dataInf.m_infs[5]);
	traceFileInf.m_traceCount = atoi(dataInf.m_infs[6]);
	traceFileInf.m_lastCandy = dataInf.m_infs[7];
	return true;
}


