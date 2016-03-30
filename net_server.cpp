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
#include "../Encrypt/verify_handel.h"
#include "../NetApp/trace_handel.h"

using namespace base;


extern CPthreadMutex g_insMutexCalc;
extern char *dataFormat;

INetServer::INetServer():SERVER_PORT(-1), m_sockLister(INVALID_SOCKET), m_nfds(0)
{
	m_listClientRead = CList::createCList();
	m_recvList = CList::createCList();
	m_newId = 0;
	return ;
}

INetServer::~INetServer()
{
}



bool INetServer::startServer()
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

    m_hListenThread = WorkThread(new boost::thread(boost::bind(&INetServer::listenThread,this)));
	m_sendThread = WorkThread(new boost::thread(boost::bind(&INetServer::sendThreadProc,this)));
	
	printf("server is start!\n");
	return true;
}


void INetServer::listenThread()
{
	int backlog = 50;
	int ret = listen(m_sockLister, backlog);
	if (ret == SOCKET_ERROR)
	{
		return ;
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
		bool needWhile = false;
		node *pHead = &m_listClientRead->head_node;
		for ((pNode)=(pHead)->next; (pHead) != (pNode);)
		{
			pClientConnRead = clientConnContain(pNode);
			needWhile = FD_ISSET(pClientConnRead->socket, &fd_read);
			while(needWhile)
			{
				RECV_DATA *pRecvData = IDealDataHandle::createRecvData();
				std::shared_ptr<CLogDataInf> &dataInf = pRecvData->calcInf.m_dataInf;
				char *&packet =  pRecvData->calcInf.m_packet;
                IParsePacket &parsePacket = pClientConnRead->clientInf->m_parsePacket;
				bool bRet = m_dataWorkManager->receiveInfData(pClientConnRead->socket, parsePacket, &packet);
				
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
			ClientConn *pClientConn = dealConnect(socket, clientAddr);
			m_dataWorkManager->openFile(*pClientConn, (char *)"Debug.cpp");
		}

	}
	return ;
}





node *INetServer::dealDisconnect(ClientConn *pClientConnRead)
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

ClientConn *INetServer::dealConnect(int socket, sockaddr_in &clientAddr)
{	trace_worker();
    char *ipAddr = NULL;
    int port = 0;            
    ipAddr = inet_ntoa(clientAddr.sin_addr);
    port = ntohs(clientAddr.sin_port);

	ClientConn *pClientConn = new ClientConn;
	std::shared_ptr<CClientInf> ptr(new CClientInf());	
	CClientInf *clientInf = ptr.get();
	pClientConn->clientInf = ptr;
	
	clientInf->m_socket = pClientConn->socket = socket;
	clientInf->m_clientId = pClientConn->clientId = creatClientId();
    clientInf->m_clientIpAddr = ipAddr;
    clientInf->m_clientPort = port;
    
	setNoBlock(clientInf->m_socket);
	m_listClientRead->push_back(&pClientConn->node);

	CUserManager::instance()->addClient(clientInf->m_clientId, clientInf);
	return pClientConn;
}

int INetServer::creatClientId()
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

void INetServer::pushRecvData(RECV_DATA *pRecvData)
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

void INetServer::sendThreadProc()
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

	return ;
}

void INetServer::dealRecvData(TimeCalcInf *pCalcInf)
{
	int &socket = pCalcInf->m_traceInfoId.socket;
	CClientInf *clientInf = pCalcInf->m_clientInf.get();

	if (socket == INVALID_SOCKET || clientInf->m_socket == INVALID_SOCKET)
	{
		return ;
	}

	char *packet = pCalcInf->m_pContent;
	int &packetLen = pCalcInf->m_contentLen;

	char sendData[16];
	int sendDataLen = 0;
	unsigned int strLenNum = packetLen + 12;
	
	memcpy(sendData+sendDataLen, (char *)"\x7B\x7B\x7B\x7B", 4);
	sendDataLen += 4;
	memcpy(sendData+sendDataLen, &strLenNum, sizeof(int));
	sendDataLen += sizeof(int);

	m_dataWorkManager->send(socket, sendData, sendDataLen);
	m_dataWorkManager->send(socket, packet, packetLen);
	m_dataWorkManager->send(socket, (char *)"\x7D\x7D\x7D\x7D", 4);
	return ;
}


void INetServer::setNoBlock(int socket)
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

CNetServer* CNetServer::_instance = NULL;

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

CNetServer::CNetServer()
{
	new CVerifyHandle;
	new CTraceHandle;

	CSimpleIniA ini;  
	ini.SetUnicode();  
	ini.LoadFile("Config.ini");  
	SERVER_PORT = (int)ini.GetLongValue("NetConfig", "NetSerPort");
	m_dataWorkManager = CDataWorkManager::create();
    return ;
}

