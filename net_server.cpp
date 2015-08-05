#include "stdafx.h"
#include "net_server.h"
#include "string_base.h"
#include "socket_base.h"
#include "data_work.h"
#include "SimpleIni.h"
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>

using namespace base;


extern CPthreadMutex g_insMutexCalc;
extern char *dataFormat;
CNetServer* CNetServer::_instance = NULL;

CNetServer::CNetServer():SERVER_PORT(-1), m_sockLister(INVALID_SOCKET), m_nfds(0)
{

	CSimpleIniA ini;  
	ini.SetUnicode();  
	ini.LoadFile("Config.ini");  
	SERVER_PORT = (int)ini.GetLongValue("NetConfig", "NetSerPort");

	m_listClientRead = CList::createCList();
	m_recvList = CList::createCList();
	m_newId = 0;
	m_dataWorkManager = CDataWorkManager::create();
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
	int nRecvBuf = 1024*256;
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
	base::pthread_create(&m_sendThread, NULL,sendThreadProc,NULL);
	
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
		if (count <= 0)
		{
			continue;
		}

		ClientConn *pClientConnRead = NULL;
		node *pHead = &m_listClientRead->head_node;
		for ((pNode)=(pHead)->next; (pHead) != (pNode);)
		{
			pClientConnRead = clientConnContain(pNode);
			while(FD_ISSET(pClientConnRead->socket, &fd_read))
			{
				RECV_DATA *pRecvData = IDealDataHandle::createRecvData();
				CLogDataInf &dataInf = pRecvData->calcInf.m_dataInf;
				bool bRet = m_dataWorkManager->receiveInfData(pClientConnRead->socket, &dataInf);
				
				if(bRet)
				{
					m_dataWorkManager->dealitemData(pClientConnRead, pRecvData);
				}
				//异常处理
				else
				{
					IDealDataHandle::destroyRecvData(pRecvData);
					pNode = m_dataWorkManager->dealErrNo(pClientConnRead, pNode);
					break;
				}
			}
			(pNode)=(pNode)->next;
		}
		
		//处理连接消息
		if(FD_ISSET(m_sockLister, &fd_read))
		{
			int socket = accept(m_sockLister,(sockaddr*)&clientAddr,&nLen);		
			if (m_nfds < socket)
			{
				m_nfds = socket;
			}
			ClientConn *pClientConn = dealConnect(socket);
			m_dataWorkManager->openFile(*pClientConn, (char *)"Debug.cpp");
		}

	}
	return NULL;
}





node *CNetServer::dealDisconnect(ClientConn *pClientConnRead)
{	trace_worker();
	CUserManager::instance()->removeClient(pClientConnRead->clientId);
	node *pNode = &pClientConnRead->node;

	CClientInf *clientInf = pClientConnRead->clientInf.get();
	clientInf->m_socket = INVALID_SOCKET;
	
	base::close(pClientConnRead->socket);	
	pNode = m_listClientRead->erase(pNode);

	delete pClientConnRead;
	return pNode;
}

ClientConn *CNetServer::dealConnect(int socket)
{	trace_worker();
	ClientConn *pClientConn = new ClientConn;
	std::shared_ptr<CClientInf> ptr(new CClientInf());	
	CClientInf *clientInf = ptr.get();
	pClientConn->clientInf = ptr;

	
	clientInf->m_socket = pClientConn->socket = socket;
	clientInf->m_clientId = pClientConn->clientId = creatClientId();

	setNoBlock(clientInf->m_socket);
	m_listClientRead->push_back(&pClientConn->node);

	CUserManager::instance()->addClient(clientInf->m_clientId, clientInf);
	return pClientConn;
}

int CNetServer::creatClientId()
{
	if (m_newId < INT_MAX)
	{
		return m_newId++;
	}
	else
	{
		m_newId = 0;
		return m_newId;
	}
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

void *CNetServer::sendThreadProc(void *arg)
{
	return CNetServer::instance()->_sendThreadProc(arg);
}

void *CNetServer::_sendThreadProc(void *arg)
{
	while(1)
	{
		if(m_recvList->empty())
		{
			base::usleep(1000);
			continue;
		}
		
		m_recvListMutex.Enter();
		struct node *pNode =  m_recvList->begin();
		RECV_DATA *pRecvData = recvDataContain(pNode);
		m_recvList->pop_front();	
		m_recvListMutex.Leave();
		
		dealRecvData(&pRecvData->calcInf);
		IDealDataHandle::destroyRecvData(pRecvData);
	}

	return NULL;
}

void CNetServer::dealRecvData(TimeCalcInf *pCalcInf)
{
	int &socket = pCalcInf->m_traceInfoId.socket;
	CClientInf *clientInf = pCalcInf->m_clientInf.get();

	if (socket == INVALID_SOCKET || clientInf->m_socket == INVALID_SOCKET)
	{
		return ;
	}

	char *packet = pCalcInf->m_pContent;
	int &packetLen = pCalcInf->m_contentLen;

	m_dataWorkManager->send(socket, packet, packetLen);
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
		perror("setNoBlock failed\n");
		return;
	}
#endif
}


