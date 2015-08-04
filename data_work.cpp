#include "stdafx.h"
#include <string.h>
#include "data_work.h"
#include "mem_base.h"
#include "data_handle.h"
#include "socket_base.h"
#include "trace_handel.h"
#include "verify_handel.h"
#include "user_manager.h"

using namespace base;
const char *dataFormat = "{\"opr\" : \"%s\", \"threadId\" : %d, \"line\" : %d, \"fileName\" : \"%s\", \"funcName\" : \"%s\", \"displayLevel\" : %d, \"content\" : \"%s\"}";

CDataWorkManager *CDataWorkManager::_instance = NULL;

CDataWorkManager::CDataWorkManager():m_errNo(e_noErr)
{
	m_workList = CList::createCList();
}


CDataWorkManager *CDataWorkManager::create()
{
	return new CDataWorkManager;
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

void CDataWorkManager::dealitemData(ClientConn *pClientConn, RECV_DATA *pRecvData)
{
	TimeCalcInf *pCalcInf = &pRecvData->calcInf;

	pCalcInf->m_traceInfoId.clientId = pClientConn->clientId;
	pCalcInf->m_traceInfoId.socket = pClientConn->socket;
	pCalcInf->m_traceInfoId.clientInf = pClientConn->clientInf.get();
	pCalcInf->m_clientInf = pClientConn->clientInf;
	CTimeCalcInfManager::instance()->pushRecvData(pRecvData);
}

void CDataWorkManager::dealWorkData(WORK_DATA *pWorkData)
{

}
void* CDataWorkManager::threadFunc(void *pArg)
{
	//CDataWorkManager::instance()->threadProc();
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


bool CDataWorkManager::receiveInfData(int socket, CLogDataInf *pDataInf)
{	trace_worker();
	const int ClenSize = 4;
	char CLen[ClenSize];
	if (receive(socket, CLen, ClenSize) <= 0)
	{
		return false;
	}
	int iLen = 0;		
	pDataInf->C2ILen(CLen,ClenSize,iLen);

	char *packet = new char[iLen];
	memcpy(packet, CLen, ClenSize);
	if (receive(socket, packet+ClenSize, iLen-ClenSize) <= 0)
	{
		return false;
	}
	pDataInf->unPacket(packet);

	return true;
}


int CDataWorkManager::receive(int fd,char *szText,int iLen)
{	trace_worker();
	int recvBufLen = 0;
	int totalRecvLen = 0;
	while (1)
	{
		recvBufLen = ::recv(fd, szText+totalRecvLen, iLen-totalRecvLen, 0);
		if (recvBufLen <= 0)
		{
			setErrNo(recvBufLen);
			if (m_errNo == e_readOk && 0 < totalRecvLen && totalRecvLen < iLen)
			{
				continue;
			}				
			return -1;
		}
		trace_str(szText+totalRecvLen, recvBufLen);
		totalRecvLen += recvBufLen;
		if (totalRecvLen == iLen)
		{
			break;
		}
	}
	return iLen;
}

int CDataWorkManager::send(int fd,char *szText,int len)
{
	int cnt;
	int rc;
	cnt=len;
	while(cnt>0)
	{
		rc=::send(fd,szText,cnt,0);
		if(rc==SOCKET_ERROR)
		{
			return -1;
		}
		if(rc==0)
		{
			return len-cnt;
		}
		szText+=rc;
		cnt-=rc;
	}
	return len;
}


void CDataWorkManager::setErrNo(int recvNum)
{
	int errNo = 0;
#ifdef WIN32
	errNo = WSAGetLastError();
	switch (errNo)
	{
		case WSAEWOULDBLOCK:
			m_errNo = e_readOk;
			break;
		case WSAECONNRESET:
			m_errNo = e_disConnect;
			break;
		default:
			m_errNo = e_noErr;
			break;
	}
#else
	errNo = errno;
	switch (errNo)
	{
		case EAGAIN:
			m_errNo = e_readOk;
			break;
		default:
			m_errNo = e_noErr;
			break;
	}
#endif
	if (recvNum == 0)
	{
		m_errNo = e_disConnect;
		return ;
	}
	return ;
}



node *CDataWorkManager::dealErrNo(ClientConn *pClientConnRead, node *pNode)
{
	switch (m_errNo)
	{
		case e_readOk:
			break;
		case e_disConnect:
			pNode = dealDisConnect(pClientConnRead, pNode);
			break;
		default:
			break;

	}
	return pNode;
}

node *CDataWorkManager::dealDisConnect(ClientConn *pClientConnRead, node *pNode)
{	trace_worker();
	CClientInf *clientInf = pClientConnRead->clientInf.get();
	CUserManager::instance()->logout(clientInf);

	dealException(*pClientConnRead);
	closeFile(*pClientConnRead);
	pNode = CNetServer::instance()->dealDisconnect(pClientConnRead);

	return pNode;
}

void CDataWorkManager::openFile(ClientConn clientConn, char *fileName)
{
	RECV_DATA *pRecvData =IDealDataHandle::createRecvData();
	
	CLogDataInf &dataInf = pRecvData->calcInf.m_dataInf;
	
	dataInf.putInf("openFile");
	dataInf.putInf("1");//session id
	dataInf.putInf("0");
	dataInf.putInf("0");
	dataInf.putInf("");
	dataInf.putInf("");
	dataInf.putInf("0");
	dataInf.putInf(fileName);

	clientConn.socket = INVALID_SOCKET;
	dealitemData(&clientConn, pRecvData); 
	return ;
}


void CDataWorkManager::closeFile(ClientConn clientConn)
{
	RECV_DATA *pRecvData = IDealDataHandle::createRecvData();
	
	CLogDataInf &dataInf = pRecvData->calcInf.m_dataInf;
	dataInf.putInf("closeFile");	
	dataInf.putInf("1");//session id
	dataInf.putInf("0");
	dataInf.putInf("0");
	dataInf.putInf("");
	dataInf.putInf("");
	dataInf.putInf("0");
	dataInf.putInf("");

	clientConn.socket = INVALID_SOCKET;
	dealitemData(&clientConn, pRecvData); 
	return ;
}


void CDataWorkManager::dealException(ClientConn clientConn)
{
	clientConn.socket = INVALID_SOCKET;
	{
		RECV_DATA *pRecvData = IDealDataHandle::createRecvData();

		CLogDataInf &dataInf = pRecvData->calcInf.m_dataInf;
		dataInf.putInf("dispAll");		
		dataInf.putInf("1");//session id
		dataInf.putInf("0");
		dataInf.putInf("0");
		dataInf.putInf("");
		dataInf.putInf("");
		dataInf.putInf("0");
		dataInf.putInf("backtrace");

		dealitemData(&clientConn, pRecvData); 
	}
	{
		RECV_DATA *pRecvData = IDealDataHandle::createRecvData();

		CLogDataInf &dataInf = pRecvData->calcInf.m_dataInf;
		dataInf.putInf("cleanAll");		
		dataInf.putInf("1");//session id
		dataInf.putInf("0");
		dataInf.putInf("0");
		dataInf.putInf("");
		dataInf.putInf("");
		dataInf.putInf("0");
		dataInf.putInf("backtrace");

		dealitemData(&clientConn, pRecvData); 
	}
	return ;
}

