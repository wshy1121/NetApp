#ifndef __DATA_WORK_H
#define __DATA_WORK_H

#include "link_tool.h"
#include "thread_base.h"
#include "time_calc.h"
#include "net_server.h"

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
	typedef enum
	{
		e_noErr,
		e_disConnect,	//socket已正常关闭
		e_readOk,		//当前缓冲区已无数据可读
		e_rst,			// 对方发送了RST
		e_intr,			// 被信号中断
	}ErrNo;
	static CDataWorkManager *instance();
	static void initDataHandle();
public:
	WORK_DATA *createWorkData(int contentLen);
	void destroyWorkData(WORK_DATA *pWorkData);
	void pushWorkData(WORK_DATA *pWorkData);	
	void dealitemData(ClientConn *pClientConn, RECV_DATA *pRecvData);	
	node *dealErrNo(ClientConn *pClientConnRead, node *pNode);
	node *dealDisConnect(ClientConn *pClientConnRead, node *pNode);
	bool receiveInfData(int socket, base::CLogDataInf *pDataInf);	
	int receive(SOCKET fd,char *szText,int len);
	int send(SOCKET fd,char *szText,int len);	
	void openFile(int fileKey, char *fileName);
	void closeFile(int fileKey);
private:
	CDataWorkManager();	
	void threadProc();
	static void* threadFunc(void *pArg);
	void dealWorkData(WORK_DATA *pWorkData);	
	void setErrNo(int recvNum);	
	void dealException(int clientId);
private:
	static CDataWorkManager *_instance;
private:
	ErrNo m_errNo;	
	base::CList *m_workList;
	base::CPthreadMutex m_workListMutex;
	base::pthread_t m_threadId;
};


#endif

