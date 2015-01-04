#ifndef __DATA_WORK_H
#define __DATA_WORK_H

#include "link_tool.h"
#include "thread_base.h"
#include "time_calc.h"

typedef struct WORK_DATA
{
	char *m_pContent;
	int m_contentLen;
	int clientId;
	struct node node;
}WORK_DATA;

#define workDataContain(ptr)  container_of(ptr, WORK_DATA, node)


class CDataWorkManager
{
public:
	static CDataWorkManager *instance();
public:
	WORK_DATA *createWorkData(int contentLen);
	void destroyWorkData(WORK_DATA *pWorkData);
	void pushWorkData(WORK_DATA *pWorkData);
private:
	CDataWorkManager();	
	void threadProc();
	static void* threadFunc(void *pArg);
	void dealitemData(int clientId, const char *itemData);	
	void dealWorkData(WORK_DATA *pWorkData);
	TimeCalcInf::TimeCalcOpr switchOpr(const char *opr);
private:
	static CDataWorkManager *_instance;
private:
	CList *m_workList;
	CPthreadMutex m_workListMutex;
	base::pthread_t m_threadId;
};


#endif

