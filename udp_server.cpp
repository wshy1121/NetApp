#include "udp_server.h"

IUdpServer::IUdpServer()
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
    return true;
}


void IUdpServer::listenThread()
{   trace_worker();

    while(true)
    {
        boost::this_thread::interruption_point();
        sleep(1);
    }
}

bool IUdpServer::receiveInfData(int socket, IParsePacket *parsePacket, std::string &packet)
{   trace_worker();
    return true;
}


