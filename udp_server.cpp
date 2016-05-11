#include <sys/types.h>
#include <sys/socket.h>
#include "udp_server.h"
#include "data_work.h"

IUdpServer::IUdpServer()
:m_maxBufferSize(4096)
{   trace_worker(); 
    return ;
}

IUdpServer::~IUdpServer()
{   trace_worker();
    
    if(m_hListenThread != NULL)
    {   trace_printf("NULL");
        m_hListenThread->interrupt();
        m_hListenThread->join();
    }
    
    if(m_sendThread != NULL)
    {   trace_printf("NULL");
        m_sendThread->interrupt();
        m_sendThread->join();
    } 
}

bool IUdpServer::startServer()
{   trace_worker();
    if(INVALID_SOCKET != m_sockLister)
	{
		printf("server is runing!\n");
		return false;
	}

	m_sockLister = socket(AF_INET, SOCK_DGRAM, 0);
	if(m_sockLister == INVALID_SOCKET)
	{
		printf("create sock error!\n");
		return false;
	}

    SERVER_PORT = getServerPort();
    trace_printf("SERVER_PORT  %d", SERVER_PORT);
	struct sockaddr_in svraddr;
	svraddr.sin_family = AF_INET;
	svraddr.sin_addr.s_addr = INADDR_ANY;
	svraddr.sin_port = htons(SERVER_PORT);

    
	int ret = bind(m_sockLister, (struct sockaddr*) &svraddr, sizeof(svraddr));
	if (ret == SOCKET_ERROR)
	{
		trace_printf("bind sock error!\n");
		printf("bind sock error!\n");
		return false;
	}
    trace_printf("NULL");

    m_dataWorkManager = createWorkManager();
    m_hListenThread = WorkThread(new boost::thread(boost::bind(&IUdpServer::listenThread,this)));
    m_sendThread = WorkThread(new boost::thread(boost::bind(&INetServer::sendThreadProc,this)));
    
    printf("udp server is start, port is %d!\n", SERVER_PORT);
    return true;
}


void IUdpServer::listenThread()
{   trace_worker();

    ClientConn clientConn;
    initClientConn(clientConn);
    while(true)
    {
        boost::this_thread::interruption_point();
        RECV_DATA *pRecvData = IDealDataHandle::createRecvData(false);
		std::string &packet =  pRecvData->calcInf.m_packet;
        IParsePacket *parsePacket = clientConn.clientInf->m_parsePacket.get();
		bool bRet = receiveInfData(clientConn.socket, parsePacket, packet);
        
        printf("packet.c_str()  %s\n", packet.c_str());
		if(bRet)
		{
			m_dataWorkManager->pushItemData(&clientConn, pRecvData);
		}
    }
}

bool IUdpServer::receiveInfData(int socket, IParsePacket *parsePacket, std::string &packet)
{   trace_worker();
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    int nRecv = 0;
    while (1)
    {
        char *charData = parsePacket->charData();
        nRecv = ::recvfrom(socket, charData, m_maxBufferSize, 0, (struct sockaddr*)&clientAddr, &addrLen);
        if (nRecv <= 0)
        {
            return false;
        }
        
        if (parsePacket->parsePacket(charData, nRecv, packet))
        {
            return true;
        }
    }

    return true;
}

void IUdpServer::initClientConn(ClientConn &clientConn)
{   trace_worker();
    std::shared_ptr<IClientInf> ptr(createClientInf());	
	IClientInf *clientInf = ptr.get();
	clientConn.clientInf = ptr;
    clientInf->m_socket = clientConn.socket = m_sockLister;
    clientInf->m_clientId = creatClientId();

    boost::shared_ptr<IParsePacket> parsePacket(createParsePacket());
    parsePacket->setClientInf(clientConn.clientInf);
    clientInf->m_parsePacket = parsePacket;
    
}

int IUdpServer::send(IClientInf *clientInf, char *szText,int len)
{
    return 0;
}

