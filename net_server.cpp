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

static boost::mutex g_insMutex;

extern CPthreadMutex g_insMutexCalc;
extern char *dataFormat;

struct event_base *INetServer::m_base = NULL;

int INetServer::m_newId = 0;

INetServer::INetServer()
:m_dataWorkManager(NULL)
,SERVER_PORT(-1)
,m_sockLister(INVALID_SOCKET)
{
#ifdef WIN32	
    WSADATA wsa={0};
    WSAStartup(MAKEWORD(2,2),&wsa);
#endif

	m_recvList = CList::createCList();

    initEvent();
	return ;
}

INetServer::~INetServer()
{
}

void INetServer::destroyClientConn(ClientConn *pClientConnRead)
{   trace_worker();


    pClientConnRead->clientInf->m_parsePacket->resetClientInf();
    IClientInf *clientInf = pClientConnRead->clientInf.get();
    clientInf->m_socket = INVALID_SOCKET;

    base::close(pClientConnRead->socket);	

    struct event *readEvent = clientInf->m_readEvent.get();
    event_del(readEvent);
    
    delete pClientConnRead;
}


ClientConn *INetServer::createClientConn(int socket, sockaddr_in *clientAddr)
{   trace_worker();

    ClientConn *pClientConn = new ClientConn;
    std::shared_ptr<IClientInf> ptr(createClientInf()); 
    IClientInf *clientInf = ptr.get();
    pClientConn->clientInf = ptr;

    clientInf->m_socket = pClientConn->socket = socket;
    clientInf->m_clientId = pClientConn->clientId = creatClientId();
    clientInf->m_clientAddr = *clientAddr;

    boost::shared_ptr<IParsePacket> parsePacket(createParsePacket());
    parsePacket->setClientInf(pClientConn->clientInf);
    clientInf->m_parsePacket = parsePacket;
    
    trace_printf("parsePacket.get()  %p", parsePacket.get());
    
    boost::shared_ptr<event> readEvent(event_new(m_base, socket, EV_READ|EV_PERSIST, doRead, pClientConn));
    if (readEvent == NULL)
    {
        destroyClientConn(pClientConn);
        return NULL;
    }
    clientInf->m_readEvent = readEvent;
    event_add(readEvent.get(), NULL);
    return pClientConn;
}

void INetServer::dealDisconnect(ClientConn *pClientConnRead)
{	trace_worker();    
    CUserManager::instance()->removeClient(pClientConnRead->clientId);

    destroyClientConn(pClientConnRead);
	return ;
}

ClientConn *INetServer::dealConnect(int socket, sockaddr_in *clientAddr)
{	trace_worker();

    evutil_make_socket_nonblocking(socket);
    ClientConn *pClientConn = createClientConn(socket, clientAddr);
    if (pClientConn == NULL)
    {   trace_printf("NULL");
        return NULL;
    }
    
    std::shared_ptr<IClientInf> &clientInf = pClientConn->clientInf;
	CUserManager::instance()->addClient(clientInf->m_clientId, clientInf.get());
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

bool INetServer::initEvent()
{   trace_worker();
    if (NULL == m_base)
    {
        boost::unique_lock<boost::mutex> lock(g_insMutex);
        if (NULL == m_base)
        {
            m_base = event_base_new();
            if (!m_base)  
            {   trace_printf(NULL);
                return false;
            }
            
        }
    }
    return true;
}

int INetServer::dispatch()
{
    event_base_dispatch(m_base);
    return 0;
}

void INetServer::doRead(evutil_socket_t clientSocket, short events, void *arg)
{   trace_worker();
    ClientConn *clientConn = (ClientConn *)arg;
    
    INetServer *netServer = clientConn->clientInf->m_netServer;
    netServer->doRead(clientConn);
    return ;
}

