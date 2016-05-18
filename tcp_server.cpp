#include "tcp_server.h"
#include "data_work.h"


bool ITcpServer::startServer()
{   trace_worker();
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
    evutil_make_socket_nonblocking(m_sockLister);
    
	int nRecvBuf = 1024*256;
	setsockopt(m_sockLister, SOL_SOCKET,SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));

	int opt = 0;
	if (setsockopt(m_sockLister, SOL_SOCKET, SO_REUSEADDR, (char*) &opt, sizeof(opt))< 0)
	{
		printf("set sock error!\n");
		return false;
	}
    SERVER_PORT = getServerPort();
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

    int backlog = 50;
	ret = listen(m_sockLister, backlog);
	if (ret == SOCKET_ERROR)
	{
		return false;
	}

	m_dataWorkManager = createWorkManager();

    struct event *listener_event = NULL;
    listener_event = event_new(m_base, m_sockLister, EV_READ|EV_PERSIST, doAccept, this);  
    event_add(listener_event, NULL);
    
	m_sendThread = WorkThread(new boost::thread(boost::bind(&INetServer::sendThreadProc,this)));
	
	printf("tcp server is start, port is %d!\n", SERVER_PORT);
	return true;
}


bool ITcpServer::doRead(ClientConn *clientConn)
{   trace_worker();
    
    RECV_DATA *pRecvData = IDealDataHandle::createRecvData(false);
    std::shared_ptr<std::string> &packet =  pRecvData->calcInf.m_packet;
    IParsePacket *parsePacket = clientConn->clientInf->m_parsePacket.get();
    
    bool bRet = receiveInfData(clientConn->socket, parsePacket, *packet.get());
	if(bRet)
	{
		m_dataWorkManager->pushItemData(clientConn, pRecvData);
	}
	else
	{
		IDealDataHandle::destroyRecvData(pRecvData);
		m_dataWorkManager->dealErrNo(clientConn);
	}
    return true;
}


bool ITcpServer::receiveInfData(int socket, IParsePacket *parsePacket, std::string &packet)
{ 
    return m_dataWorkManager->receiveInfData(socket, parsePacket, packet);
}

int ITcpServer::send(IClientInf *clientInf, char *szText,int len)
{
    return m_dataWorkManager->send(clientInf->m_socket, szText, len);
}

void ITcpServer::doAccept(evutil_socket_t listenerSocket, short event, void *arg)
{   trace_worker();
    INetServer *netServer = (INetServer *)arg;
    
    struct sockaddr_storage clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    
    int clientSocket = accept(listenerSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);  
    if (clientSocket < 0)  
    {
        perror("accept");  
    }
    
    else if (clientSocket > FD_SETSIZE)  
    {  
        close(clientSocket);// XXX replace all closes with EVUTIL_CLOSESOCKET */  
    }  
    else  
    {
        netServer->dealConnect(clientSocket, (sockaddr_in *)&clientAddr);
    }  
}





