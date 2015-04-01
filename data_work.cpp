#include "stdafx.h"
#include <string.h>
#include "data_work.h"
#include "mem_base.h"
#include "data_handle.h"
#include "Global.h"

using namespace base;
const char *dataFormat = "{\"opr\" : \"%s\", \"threadId\" : %d, \"line\" : %d, \"fileName\" : \"%s\", \"funcName\" : \"%s\", \"displayLevel\" : %d, \"content\" : \"%s\"}";

extern CPthreadMutex g_insMutexCalc;

CDataWorkManager *CDataWorkManager::_instance = NULL;

CDataWorkManager::CDataWorkManager()
{
	m_workList = CList::createCList();
	IDealDataHandle::initDataHandle();
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

void CDataWorkManager::dealitemData(int clientId, char *infs[], int totalLen, int infLens[])
{
	char *oper = infs[0];
	int operLen = infLens[0];
	base::pthread_t threadId = atoi(infs[1]);
	int line = atoi(infs[2]);
	char *file_name = infs[3];
	int fileLen = infLens[3] + 1;
	char *funcName = infs[4];
	int funcLen = infLens[4] + 1;
	int display_level = atoi(infs[5]);
	const char *traceContent = infs[6];
	int contentLen = infLens[6] + 1;

	RECV_DATA *pRecvData = CTimeCalcInfManager::instance()->createRecvData(totalLen);	
	TimeCalcInf *pCalcInf = &pRecvData->calcInf;
	
	int memBufPos = 0;
	for(int i=0; infs[i] != NULL; ++i)
	{
		pCalcInf->infs[i] = pCalcInf->m_memBuffer + memBufPos;
		memcpy(pCalcInf->infs[i], infs[i], infLens[i]);
		pCalcInf->infLens[i] = infLens[i];
		memBufPos += infLens[i];
	}

	pCalcInf->m_oper = pCalcInf->infs[0];
	pCalcInf->m_fileName = pCalcInf->infs[3];
	pCalcInf->m_funcName = pCalcInf->infs[4];
	pCalcInf->m_pContent = pCalcInf->infs[6];
		
	pCalcInf->m_traceInfoId.threadId = threadId;
	pCalcInf->m_traceInfoId.clientId = clientId;
	pCalcInf->m_line = line;
	pCalcInf->m_displayLevel = display_level;

	CTimeCalcInfManager::instance()->pushRecvData(pRecvData);
}

void CDataWorkManager::dealWorkData(WORK_DATA *pWorkData)
{

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









