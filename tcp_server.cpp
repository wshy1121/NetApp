#include "tcp_server.h"
#include "data_work.h"


bool ITcpServer::startServer()
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
	m_nfds = m_sockLister;

	m_dataWorkManager = createWorkManager();
    m_hListenThread = WorkThread(new boost::thread(boost::bind(&ITcpServer::listenThread,this)));
	m_sendThread = WorkThread(new boost::thread(boost::bind(&INetServer::sendThreadProc,this)));
	
	printf("tcp server is start, port is %d!\n", SERVER_PORT);
	return true;
}


void ITcpServer::listenThread()
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
		node *pHead = &m_listClientRead->head_node;
		for ((pNode)=(pHead)->next; (pHead) != (pNode);)
		{
			pClientConnRead = clientConnContain(pNode);

			bool &isBackClient = pClientConnRead->clientInf->m_isBackClient;
			RECV_DATA *pRecvData = IDealDataHandle::createRecvData(false);
			std::shared_ptr<std::string> &packet =  pRecvData->calcInf.m_packet;
            IParsePacket *parsePacket = pClientConnRead->clientInf->m_parsePacket.get();
			bool bRet = receiveInfData(pClientConnRead->socket, parsePacket, *packet.get());
			if(bRet && !isBackClient)
			{
				m_dataWorkManager->pushItemData(pClientConnRead, pRecvData);
			}
			else
			{
				IDealDataHandle::destroyRecvData(pRecvData);
				pNode = m_dataWorkManager->dealErrNo(pClientConnRead, pNode);
			}
			(pNode)=(pNode)->next;
		}
		
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

bool ITcpServer::receiveInfData(int socket, IParsePacket *parsePacket, std::string &packet)
{ 
    return m_dataWorkManager->receiveInfData(socket, parsePacket, packet);
}

int ITcpServer::send(IClientInf *clientInf, char *szText,int len)
{
    return m_dataWorkManager->send(clientInf->m_socket, szText, len);
}

