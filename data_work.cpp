#include "stdafx.h"
#include "data_work.h"
#include "mem_base.h"
#include "open_src.h"
#include "Global.h"

extern CPthreadMutex g_insMutexCalc;

CDataWorkManager *CDataWorkManager::_instance = NULL;

CDataWorkManager::CDataWorkManager()
{
	m_workList = CList::createCList();
	base::pthread_create(&m_threadId, NULL,threadFunc,NULL);
}

CDataWorkManager *CDataWorkManager::instance()
{
	if (NULL == _instance)
	{
		CGuardMutex guardMutex(g_insMutexCalc);
		if (NULL == _instance)
		{
			_instance = new CDataWorkManager;
		}
	}
	return _instance;
}


WORK_DATA *CDataWorkManager::createWorkData(int contentLen)
{
	WORK_DATA *pWorkData = (WORK_DATA *)base::malloc(sizeof(WORK_DATA));
	pWorkData->m_pContent = NULL;
	pWorkData->m_contentLen = 0;

	if (contentLen > 0)
	{
		pWorkData->m_pContent = (char *)base::malloc(contentLen+1);		
		pWorkData->m_pContent[contentLen] = '\0';
		pWorkData->m_contentLen = contentLen;
	}
	return pWorkData;
}

void CDataWorkManager::destroyWorkData(WORK_DATA *pWorkData)
{
	char *pContent = pWorkData->m_pContent;
	int contentLen = pWorkData->m_contentLen;

	if (contentLen > 0 && pContent != NULL)
	{
		base::free(pContent);
	}
	base::free(pWorkData);
}


void CDataWorkManager::threadProc()
{	
	while(1)
	{

		if(m_workList->empty())
		{
			base::usleep(10 * 1000);
			continue;
		}
		m_workListMutex.Enter();
		struct node *pNode =  m_workList->begin();
		WORK_DATA *pWorkData = workDataContain(pNode);
		m_workList->pop_front();	
		m_workListMutex.Leave();
		dealWorkData(pWorkData);
		destroyWorkData(pWorkData);
	}
}

void CDataWorkManager::dealitemData(int clientId, const char *itemData)
{
	Json::Reader reader;
	Json::Value traceInf;
	
	if (!reader.parse(itemData, traceInf))
	{
		printf("reader.parse fained!!!itemData:%s", itemData);
	}
	TimeCalcInf::TimeCalcOpr opr = switchOpr(traceInf["opr"].asCString());
	base::pthread_t threadId = traceInf["threadId"].asInt();
	int line = traceInf["line"].asInt();
	char *file_name = (char *)traceInf["fileName"].asCString();
	char *funcName = (char *)traceInf["funcName"].asCString();
	int display_level = traceInf["displayLevel"].asInt();
	const char *traceContent = traceInf["content"].asCString();

	int contentLen = strlen(traceContent) + 1;
	int fileLen = strlen(file_name) + 1;
	int funcLen = strlen(funcName) + 1;
	int dataLen = contentLen + fileLen + funcLen;
	RECV_DATA *pRecvData = CTimeCalcInfManager::instance()->createRecvData(dataLen);
	TimeCalcInf *pCalcInf = &pRecvData->calcInf;
	pCalcInf->m_fileName = pCalcInf->m_pContent + contentLen;
	pCalcInf->m_funcName = pCalcInf->m_pContent + contentLen + fileLen;

	pCalcInf->m_opr = opr;
	pCalcInf->m_traceInfoId.threadId = threadId;
	pCalcInf->m_traceInfoId.clientId = clientId;
	pCalcInf->m_line = line;
	pCalcInf->m_displayLevel = display_level;
	
	base::strcpy(pCalcInf->m_fileName, file_name);
	base::strcpy(pCalcInf->m_funcName, funcName);
	base::strcpy(pCalcInf->m_pContent, traceContent);
	CTimeCalcInfManager::instance()->pushRecvData(pRecvData);
}

void CDataWorkManager::dealWorkData(WORK_DATA *pWorkData)
{
	const char *recvBuf = pWorkData->m_pContent;
	int recvLen = pWorkData->m_contentLen;
	
	int beginIndex = -1;
	for (int i=0; i<recvLen; ++i)
	{
		if (recvBuf[i] == '{')
		{
			beginIndex = i;
		}
		else if (recvBuf[i] == '}' && beginIndex != -1)
		{							
			dealitemData(pWorkData->clientId, recvBuf+beginIndex);				
			beginIndex = -1;
		}
	}
}
void* CDataWorkManager::threadFunc(void *pArg)
{
	CDataWorkManager::instance()->threadProc();
	return NULL;
}


void CDataWorkManager::pushWorkData(WORK_DATA *pWorkData)
{
	if (pWorkData == NULL)
	{
		return ;
	}
	m_workListMutex.Enter();
	m_workList->push_back(&pWorkData->node);
	m_workListMutex.Leave();

}

TimeCalcInf::TimeCalcOpr CDataWorkManager::switchOpr(const char *opr)
{
	TimeCalcInf::TimeCalcOpr oprInt = TimeCalcInf::e_none;
	if (strcmp(opr, "createCandy") == 0)
	{
		oprInt = TimeCalcInf::e_createCandy;
	}
	else if (strcmp(opr, "destroyCandy") == 0)
	{
		oprInt = TimeCalcInf::e_destroyCandy;
	}
	else if (strcmp(opr, "insertTrace") == 0)
	{
		oprInt = TimeCalcInf::e_insertTrace;
	}
	else if (strcmp(opr, "dispAll") == 0)
	{
		oprInt = TimeCalcInf::e_dispAll;
	}
	else if (strcmp(opr, "insertTag") == 0)
	{
		oprInt = TimeCalcInf::e_insertTag;
	}
	else if (strcmp(opr, "printfMemInfMap") == 0)
	{
		oprInt = TimeCalcInf::e_printfMemInfMap;
	}
	else
	{
		printf("un ligical opr  %s\n", opr);
	}

	return oprInt;
}

