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

CDataWorkManager::CDataWorkManager()
:m_errNo(e_noErr)
,m_headCount(0)
,m_tailCount(0)
,m_packetPos(0)
,m_curPacketSize(0)
,m_maxBufferSize(1024*1024)

{
	m_workList = CList::createCList();
	m_packetBuffer = new char[m_maxBufferSize];
}

CDataWorkManager::~CDataWorkManager()
{
	delete []m_packetBuffer;
	CList::destroyClist(m_workList);
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


bool CDataWorkManager::receiveInfData(int socket, CParsePacket &parsePacket, char **pPacket)
{ 

	int nRecv = 0;
	
	while (1)
	{
		char &charData = parsePacket.charData();
		nRecv = ::recv(socket, &charData, 1, 0);
		if (nRecv <= 0)
		{
			setErrNo(nRecv);	
			return false;
		}
        
        if (parsePacket.parsePacket(charData, pPacket))
        {
            return true;
        }
	}

	return true;
}

int CDataWorkManager::receive(int fd,char *szText,int iLen)
{
	int recvBufLen = 0;
	int totalRecvLen = 0;
	while (1)
	{
		recvBufLen = ::recv(fd, szText+totalRecvLen, iLen-totalRecvLen, 0);
		if (recvBufLen <= 0)
		{
			setErrNo(recvBufLen);
			if (totalRecvLen == 0)
			{
				return -1;
			}
			if (m_errNo == e_readOk)
			{
				continue;
			}				
			return -1;
		}
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
{
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
	
	std::shared_ptr<CLogDataInf> &dataInf = pRecvData->calcInf.m_dataInf;
	
	dataInf->putInf("openFile");
	dataInf->putInf("1");//session id
	dataInf->putInf("0");
	dataInf->putInf("0");
	dataInf->putInf("");
	dataInf->putInf("");
	dataInf->putInf("0");
	dataInf->putInf(fileName);

	clientConn.socket = INVALID_SOCKET;
	dealitemData(&clientConn, pRecvData); 
	return ;
}


void CDataWorkManager::closeFile(ClientConn clientConn)
{
	RECV_DATA *pRecvData = IDealDataHandle::createRecvData();
	
	std::shared_ptr<CLogDataInf> &dataInf = pRecvData->calcInf.m_dataInf;
	dataInf->putInf("closeFile");	
	dataInf->putInf("1");//session id
	dataInf->putInf("0");
	dataInf->putInf("0");
	dataInf->putInf("");
	dataInf->putInf("");
	dataInf->putInf("0");
	dataInf->putInf("");

	clientConn.socket = INVALID_SOCKET;
	dealitemData(&clientConn, pRecvData); 
	return ;
}


void CDataWorkManager::dealException(ClientConn clientConn)
{
	clientConn.socket = INVALID_SOCKET;
	{
		RECV_DATA *pRecvData = IDealDataHandle::createRecvData();

		std::shared_ptr<CLogDataInf> &dataInf = pRecvData->calcInf.m_dataInf;
		dataInf->putInf("dispAll");		
		dataInf->putInf("1");//session id
		dataInf->putInf("0");
		dataInf->putInf("0");
		dataInf->putInf("");
		dataInf->putInf("");
		dataInf->putInf("0");
		dataInf->putInf("backtrace");

		dealitemData(&clientConn, pRecvData); 
	}
	{
		RECV_DATA *pRecvData = IDealDataHandle::createRecvData();

		std::shared_ptr<CLogDataInf> &dataInf = pRecvData->calcInf.m_dataInf;
		dataInf->putInf("cleanAll");		
		dataInf->putInf("1");//session id
		dataInf->putInf("0");
		dataInf->putInf("0");
		dataInf->putInf("");
		dataInf->putInf("");
		dataInf->putInf("0");
		dataInf->putInf("backtrace");

		dealitemData(&clientConn, pRecvData); 
	}
	return ;
}

