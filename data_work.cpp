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

IDataWorkManager::IDataWorkManager()
:m_errNo(e_noErr)
,m_headCount(0)
,m_tailCount(0)
,m_packetPos(0)
,m_curPacketSize(0)
,m_maxBufferSize(1024*1024)
,m_netServer(NULL)

{
	m_recvList = CList::createCList();
	m_packetBuffer = new char[m_maxBufferSize];

    m_threadId = WorkThread(new boost::thread(boost::bind(&IDataWorkManager::threadProc,this)));
}

IDataWorkManager::~IDataWorkManager()
{
	delete []m_packetBuffer;
	CList::destroyClist(m_recvList);
}

void IDataWorkManager::threadProc()
{	
	while(1)
	{
		if(m_recvList->empty())
		{
			base::usleep(10 * 1000);
			continue;
		}
        trace_worker();
		m_recvListMutex.Enter();
		struct node *pNode =  m_recvList->begin();
		RECV_DATA *pRecvData = recvDataContain(pNode);
        dealitemData(NULL, pRecvData);
		m_recvList->pop_front();	
		m_recvListMutex.Leave();
	}
}

void IDataWorkManager::pushItemData(ClientConn *pClientConn, RECV_DATA *pRecvData)
{   trace_worker();
	if (pRecvData == NULL)
	{
		return ;
	}

	TimeCalcInf *pCalcInf = &pRecvData->calcInf;
	pCalcInf->m_traceInfoId.clientId = pClientConn->clientId;
	pCalcInf->m_traceInfoId.socket = pClientConn->socket;
	pCalcInf->m_traceInfoId.clientInf = pClientConn->clientInf.get();
	pCalcInf->m_clientInf = pClientConn->clientInf;
    
	m_recvListMutex.Enter();
	m_recvList->push_back(&pRecvData->node);
	m_recvListMutex.Leave();

}


bool IDataWorkManager::receiveInfData(int socket, IParsePacket *parsePacket, char **pPacket)
{ 

	int nRecv = 0;
	
	while (1)
	{
		char &charData = parsePacket->charData();
		nRecv = ::recv(socket, &charData, 1, 0);
		if (nRecv <= 0)
		{
			setErrNo(nRecv);	
			return false;
		}
        
        if (parsePacket->parsePacket(charData, pPacket))
        {
            return true;
        }
	}

	return true;
}

int IDataWorkManager::receive(int fd,char *szText,int iLen)
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

int IDataWorkManager::send(int fd,char *szText,int len)
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


void IDataWorkManager::setErrNo(int recvNum)
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



node *IDataWorkManager::dealErrNo(ClientConn *pClientConnRead, node *pNode)
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

node *IDataWorkManager::dealDisConnect(ClientConn *pClientConnRead, node *pNode)
{
	IClientInf *clientInf = pClientConnRead->clientInf.get();
	CUserManager::instance()->logout(clientInf);

	dealException(*pClientConnRead);
	closeFile(*pClientConnRead);
	pNode = m_netServer->dealDisconnect(pClientConnRead);

	return pNode;
}

void IDataWorkManager::openFile(ClientConn clientConn, char *fileName)
{
	return ;
}


void IDataWorkManager::closeFile(ClientConn clientConn)
{
	return ;
}


void IDataWorkManager::dealException(ClientConn clientConn)
{
	return ;
}




