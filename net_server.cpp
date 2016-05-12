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

INetServer::INetServer()
:m_dataWorkManager(NULL)
,SERVER_PORT(-1)
,m_sockLister(INVALID_SOCKET)
,m_nfds(0)
{
#ifdef WIN32	
    WSADATA wsa={0};
    WSAStartup(MAKEWORD(2,2),&wsa);
#endif

	m_listClientRead = CList::createCList();
	m_recvList = CList::createCList();
	m_newId = 0;
	return ;
}

INetServer::~INetServer()
{
}

node *INetServer::dealDisconnect(ClientConn *pClientConnRead)
{	trace_worker();
	CUserManager::instance()->removeClient(pClientConnRead->clientId);
	node *pNode = &pClientConnRead->node;

    pClientConnRead->clientInf->m_parsePacket->resetClientInf();
	IClientInf *clientInf = pClientConnRead->clientInf.get();
	clientInf->m_socket = INVALID_SOCKET;
	
	base::close(pClientConnRead->socket);	
	pNode = m_listClientRead->erase(pNode);

	delete pClientConnRead;
	return pNode;
}

ClientConn *INetServer::dealConnect(int socket, sockaddr_in &clientAddr)
{	trace_worker();

	ClientConn *pClientConn = new ClientConn;
	std::shared_ptr<IClientInf> ptr(createClientInf());	
	IClientInf *clientInf = ptr.get();
	pClientConn->clientInf = ptr;
	
	clientInf->m_socket = pClientConn->socket = socket;
	clientInf->m_clientId = pClientConn->clientId = creatClientId();
    clientInf->m_clientAddr = clientAddr;

    boost::shared_ptr<IParsePacket> parsePacket(createParsePacket());
    parsePacket->setClientInf(pClientConn->clientInf);
    clientInf->m_parsePacket = parsePacket;

	setNoBlock(clientInf->m_socket);
	m_listClientRead->push_back(&pClientConn->node);

	CUserManager::instance()->addClient(clientInf->m_clientId, clientInf);
	return pClientConn;
}

IClientInf *INetServer::createClientInf()
{
    return new IClientInf(this);
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
		
		m_dataWorkManager->dealSendData(&pRecvData->calcInf);
		IDealDataHandle::destroyRecvData(pRecvData);
	}

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


