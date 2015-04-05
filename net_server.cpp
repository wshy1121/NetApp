#include "stdafx.h"
#include "net_server.h"
#include "string_base.h"
#include "socket_base.h"
#include "data_work.h"
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "Global.h"

using namespace base;


extern CPthreadMutex g_insMutexCalc;
extern char *dataFormat;
CNetServer* CNetServer::_instance = NULL;

CNetServer::CNetServer():SERVER_PORT(8889), m_errNo(e_noErr), m_sockLister(INVALID_SOCKET), m_recvBufLen(1024*1024), m_nfds(0)
{
	m_recvBuf = (char *)base::malloc(m_recvBufLen);
	m_listClientRead = CList::createCList();
	m_recvList = CList::createCList();
	m_cientIds.reset();
	m_newId = 0;
	return ;
}

CNetServer* CNetServer::instance() 
{	
	if (NULL == _instance)
	{
		CGuardMutex guardMutex(g_insMutexCalc);
		if (NULL == _instance)
		{
#ifdef WIN32	
			WSADATA wsa={0};
			WSAStartup(MAKEWORD(2,2),&wsa);
#endif
			_instance = new CNetServer;
		}
	}
	return _instance;
}

bool CNetServer::startServer()
{
	if(INVALID_SOCKET != m_sockLister)
	{
		printf("server is runing!\n");
		return false;
	}
	
	m_sockLister = socket(AF_INET, SOCK_STREAM, 0);
	if(m_sockLister == INVALID_SOCKET)
	{
		printf("create sock error!\n");
		return false;
	}
	int nRecvBuf = m_recvBufLen;
	setsockopt(m_sockLister, SOL_SOCKET,SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));

	int opt = 0;
	if (setsockopt(m_sockLister, SOL_SOCKET, SO_REUSEADDR, (char*) &opt, sizeof(opt))< 0)
	{
		printf("set sock error!\n");
		return false;
	}
	struct sockaddr_in svraddr;
	svraddr.sin_family = AF_INET;
	svraddr.sin_addr.s_addr = INADDR_ANY;
	svraddr.sin_port = htons(SERVER_PORT);

	int ret = bind(m_sockLister, (struct sockaddr*) &svraddr, sizeof(svraddr));
	if (ret == SOCKET_ERROR)
	{
		printf("bind sock error!\n");
		return false;
	}
	m_nfds = m_sockLister;

	base::pthread_create(&m_hListenThread, NULL,listenThread,NULL);

	printf("server is start!\n");
	return true;
}


void *CNetServer::listenThread(void *arg)
{
	return CNetServer::instance()->_listenThread(arg);
}


void *CNetServer::_listenThread(void *arg)
{
	int backlog = 50;
	int ret = listen(m_sockLister, backlog);
	if (ret == SOCKET_ERROR)
	{
		return NULL;
	}
	fd_set fd_read, fd_write;
	sockaddr_in clientAddr;
	socklen_t nLen = sizeof(sockaddr);
	
	node *pNode = NULL;
	while(true)
	{
		struct timeval cctv = {0, 5000};
		FD_ZERO(&fd_read);
		FD_ZERO(&fd_write);
		FD_SET(m_sockLister, &fd_read);

		ClientConn *pClientConnTmp = NULL;
		each_link_node(&m_listClientRead->head_node, pNode)
		{
			pClientConnTmp = clientConnContain(pNode);
			FD_SET(pClientConnTmp->socket, &fd_read);
		}
		
		int count = select(m_nfds+1, &fd_read, &fd_write, NULL, &cctv);
		for (int i=0; i< count; ++i)
		{
			ClientConn *pClientConnRead = NULL;
			node *pHead = &m_listClientRead->head_node;
			for ((pNode)=(pHead)->next; (pHead) != (pNode);)
			{
				pClientConnRead = clientConnContain(pNode);
				while(FD_ISSET(pClientConnRead->socket, &fd_read))
				{
					RECV_DATA *pRecvData = IDealDataHandle::createRecvData();
					CLogDataInf &dataInf = pRecvData->calcInf.m_dataInf;
					bool bRet = receiveInfData(pClientConnRead->socket, &dataInf);
					
					if(bRet)
					{
						CDataWorkManager::instance()->dealitemData(pClientConnRead->clientId, pRecvData);
					}
					//异常处理
					else
					{
						IDealDataHandle::destroyRecvData(pRecvData);
						pNode = dealErrNo(pClientConnRead, pNode);
						break;
					}
				}
				(pNode)=(pNode)->next;
			}
			
			//处理连接消息
			if(FD_ISSET(m_sockLister, &fd_read))
			{
				SOCKET socket = accept(m_sockLister,(sockaddr*)&clientAddr,&nLen);		
				if (m_nfds < socket)
				{
					m_nfds = socket;
				}
				ClientConn *pClientConn = dealConnect(socket);
				openFile(pClientConn->clientId, (char *)"Debug.cpp");
			}
		}
		sendThreadProc();
	}
	return NULL;
}
void CNetServer::openFile(int fileKey, char *fileName)
{
	RECV_DATA *pRecvData =IDealDataHandle::createRecvData();
	
	CLogDataInf &dataInf = pRecvData->calcInf.m_dataInf;
	
	dataInf.putInf("openFile");
	dataInf.putInf("0");
	dataInf.putInf("0");
	dataInf.putInf("");
	dataInf.putInf("");
	dataInf.putInf("0");
	dataInf.putInf(fileName);
		
	CDataWorkManager::instance()->dealitemData(fileKey, pRecvData); 
	return ;
}


void CNetServer::closeFile(int fileKey)
{
	RECV_DATA *pRecvData = IDealDataHandle::createRecvData();
	
	CLogDataInf &dataInf = pRecvData->calcInf.m_dataInf;
	dataInf.putInf("closeFile");
	dataInf.putInf("0");
	dataInf.putInf("0");
	dataInf.putInf("");
	dataInf.putInf("");
	dataInf.putInf("0");
	dataInf.putInf("");

	CDataWorkManager::instance()->dealitemData(fileKey, pRecvData); 
	return ;
}


void CNetServer::dealException(int clientId)
{
	{
		RECV_DATA *pRecvData = IDealDataHandle::createRecvData();

		CLogDataInf &dataInf = pRecvData->calcInf.m_dataInf;
		dataInf.putInf("dispAll");
		dataInf.putInf("0");
		dataInf.putInf("0");
		dataInf.putInf("");
		dataInf.putInf("");
		dataInf.putInf("0");
		dataInf.putInf("backtrace");

		CDataWorkManager::instance()->dealitemData(clientId, pRecvData); 
	}
	{
		RECV_DATA *pRecvData = IDealDataHandle::createRecvData();

		CLogDataInf &dataInf = pRecvData->calcInf.m_dataInf;
		dataInf.putInf("cleanAll");
		dataInf.putInf("0");
		dataInf.putInf("0");
		dataInf.putInf("");
		dataInf.putInf("");
		dataInf.putInf("0");
		dataInf.putInf("backtrace");

		CDataWorkManager::instance()->dealitemData(clientId, pRecvData); 
	}
	return ;
}

node *CNetServer::dealDisconnect(ClientConn *pClientConnRead)
{
	node *pNode = &pClientConnRead->node;
	
	resetClientId(pClientConnRead->clientId);
	base::close(pClientConnRead->socket);	
	pNode = m_listClientRead->erase(pNode); 					
	base::free(pClientConnRead);
	return pNode;
}

ClientConn *CNetServer::dealConnect(int clientId)
{
	ClientConn *pClientConn = (ClientConn *)base::malloc(sizeof(ClientConn));
	pClientConn->socket = clientId;
	pClientConn->clientId = creatClientId();
	
	setClientId(pClientConn->clientId);
	m_listClientRead->push_back(&pClientConn->node);
	setNoBlock(pClientConn->socket);
	return pClientConn;
}

bool CNetServer::receiveInfData(int socket, CLogDataInf *pDataInf)
{
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

int CNetServer::receive(SOCKET fd,char *szText,int iLen)
{
	int recvBufLen = 0;
	int totalRecvLen = 0;
	while (1)
	{
		recvBufLen = recv(fd, szText+totalRecvLen, iLen-totalRecvLen, 0);
		if (recvBufLen <= 0)
		{
			setErrNo(recvBufLen);
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

int CNetServer::send(SOCKET fd,char *szText,int len)
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

int CNetServer::creatClientId()
{
	if (m_newId < MAX_CLIENT_SIZE)
	{
		return m_newId++;
	}
	else
	{
		m_newId = 0;
		return m_newId;
	}
}

void CNetServer::setClientId(int clientId)
{
	m_cientIds.set(clientId);
	return ;
}

void CNetServer::resetClientId(int clientId)
{
	m_cientIds.reset(clientId);
	return ;
}

void CNetServer::pushRecvData(RECV_DATA *pRecvData)
{
	if (pRecvData == NULL)
	{
		return ;
	}
	
	m_recvListMutex.Enter();
	m_recvList->push_back(&pRecvData->node);
	m_recvListMutex.Leave();
	return ;
}

void CNetServer::sendThreadProc()
{
	while (!m_recvList->empty())
	{
		m_recvListMutex.Enter();
		struct node *pNode =  m_recvList->begin();
		RECV_DATA *pRecvData = recvDataContain(pNode);
		m_recvList->pop_front();	
		m_recvListMutex.Leave();
		
		dealRecvData(&pRecvData->calcInf);
		IDealDataHandle::destroyRecvData(pRecvData);
	}
}

void CNetServer::dealRecvData(TimeCalcInf *pCalcInf)
{
	CLogDataInf &dataInf = pCalcInf->m_dataInf;
	int &clientId = pCalcInf->m_traceInfoId.clientId;
	printf("dealRecvData  m_cientIds %d  %d\n", m_cientIds.test(clientId), clientId);
	if (!m_cientIds.test(clientId))
	{
		return ;
	}
	
	char *packet = NULL;
	int packetLen = dataInf.packet(packet);
	dataInf.packet(packet);

	send(clientId, packet, packetLen);
	printf("packetLen  %d  %s\n", packetLen, packet);
	return ;
}


void CNetServer::setNoBlock(int socket)
{
#ifdef WIN32
	unsigned long ul = 1;
	int ret = ioctlsocket(socket, FIONBIO, (unsigned long*)&ul);
	if (ret == SOCKET_ERROR)
	{
		printf("setNoBlock failed  %d\n", socket);
		return;
	}
#else
	int opts;
	opts=fcntl(socket,F_GETFL);
	if(opts<0)
	{
		perror("fcntl(sock,GETFL)");
		return;
	}
	opts = opts|O_NONBLOCK;
	if(fcntl(socket,F_SETFL,opts)<0)
	{
		perror("setNoBlock failed  %d\m", socket);
		return;
	}
#endif
}


void CNetServer::setErrNo(int recvNum)
{
	int errNo = 0;
#ifdef WIN32
	errNo = WSAGetLastError();
	printf("setErrNo errNo	%d\n", errNo);
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



node *CNetServer::dealErrNo(ClientConn *pClientConnRead, node *pNode)
{
	int errNo = errno;
	switch (m_errNo)
	{
		case e_readOk:
			break;
		case e_disConnect:
			dealException(pClientConnRead->clientId);
			closeFile(pClientConnRead->clientId);
			pNode = dealDisconnect(pClientConnRead);			
			break;
		default:
			break;

	}
	return pNode;
}

