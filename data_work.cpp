#include "stdafx.h"
#include "data_work.h"
#include "mem_base.h"
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
		WORK_DATA *pRecvData = workDataContain(pNode);
		m_workList->pop_front();	
		m_workListMutex.Leave();
		time_printf("pRecvData->m_pContent  %s\n", pRecvData->m_pContent);
		//dealRecvData(&pRecvData->calcInf);
		destroyWorkData(pRecvData);
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

