#include <boost/thread/mutex.hpp>
#include "cli_server.h"
#include "link_tool.h"
#include "verify_handel.h"
#include "trace_handel.h"
#include "SimpleIni.h"
#include "CmdLineInter.h"

extern FILE *rl_instream;
extern FILE *rl_outstream;

static boost::mutex g_insMutex;

CCliManager::CCliManager(INetServer* const netServer)
{
    m_netServer = netServer;
}


void CCliManager::dealException(ClientConn clientConn)
{
    printf("CCliTcpServer client disconnect socket:%d clientId:%d\n", clientConn.socket, clientConn.clientId);
}

void CCliManager::dealSendData(TimeCalcInf *pCalcInf)
{   trace_worker();
    IClientInf *clientInf = pCalcInf->m_clientInf.get();
    int &socket = clientInf->m_socket;
    if (socket == INVALID_SOCKET)
    {
        return ;
    }
    std::shared_ptr<std::string> &packet = pCalcInf->m_packet;
    clientInf->m_netServer->send(clientInf, (char *)packet->c_str(), packet->size());
}

void CCliManager::dealitemData(RECV_DATA *pRecvData)
{   trace_worker();
    trace_printf("%s  ", pRecvData->calcInf.m_packet->c_str());
    std::shared_ptr<std::string> &packet = pRecvData->calcInf.m_packet;
    pRecvData->calcInf.m_clientInf->m_parsePacket->writeData((char *)packet->c_str(), packet->size());
}

CCliParsePacket::CCliParsePacket()
{   trace_worker();
	pipe(m_instream);
	m_instreamFile = fdopen(m_instream[0], "r");
	
	pipe(m_outstream);
	m_outstreamFile = fdopen(m_outstream[1], "w");

	m_cliThread = DataWorkThread(new boost::thread(boost::bind(&CCliParsePacket::cliThreadProc,this)));	
	m_sendThread = DataWorkThread(new boost::thread(boost::bind(&CCliParsePacket::sendThreadProc,this)));	
}

CCliParsePacket::~CCliParsePacket()
{   trace_worker();  
    close(m_instream[1]);
    fclose(m_instreamFile);
    trace_printf("NULL");
    close(m_outstream[0]);
    fclose(m_outstreamFile);
    
    trace_printf("NULL");
    if(m_cliThread != NULL)
    {   trace_printf("NULL");
        m_cliThread->interrupt();
        m_cliThread->join();
    }
    trace_printf("NULL");
    if(m_sendThread != NULL)
    {   trace_printf("NULL");
        m_sendThread->interrupt();
        m_sendThread->join();
    }
    trace_printf("NULL");    
}


bool CCliParsePacket::parsePacket(char *charData, int charDataLen, std::string &packet)
{   trace_worker();
    trace_printf("charData  %s|", charData);
    packet.push_back(charData[0]);
    
    return true;
}



void CCliParsePacket::initPacketInf()
{
}

void CCliParsePacket::writeData(char *data, int dataLen)
{   trace_worker();
	rl_instream = m_instreamFile;
	rl_outstream = m_outstreamFile;
	write (m_instream[1], data, dataLen);

}

void CCliParsePacket::cliThreadProc()
{
	char *line, *s;
	initialize_readline();
	while(1)
	{
		boost::this_thread::interruption_point();

		rl_instream = m_instreamFile;
		rl_outstream = m_outstreamFile;
        line = readline("\r> ");
        if (!line)
    	{
			break;
		}
        s = strstrip(line);
        if (*s) {
            add_history(s);
            execute_line(s);
        }
	}

}

void CCliParsePacket::sendThreadProc()
{
	char data[1024];
	int dataLen = 0;
	while(1)
	{
	    boost::this_thread::interruption_point();
		dataLen = read (m_outstream[0], data, sizeof(data));		
		if (dataLen <= 0)
		{
			break;
		}
		data[dataLen] = '\0';
        if (m_clientInf != NULL)
        {
            RECV_DATA *repRecvData = packetRecvData(data);
            INetServer *&netServer = m_clientInf->m_netServer;      
            netServer->pushRecvData(repRecvData);
        }
	}
}

RECV_DATA *CCliParsePacket::packetRecvData(char *data)
{
	RECV_DATA *recvData = IDealDataHandle::createRecvData();

    std::string &packet = *(recvData->calcInf.m_packet.get());
	packet = data;
	recvData->calcInf.m_clientInf = m_clientInf;
	return recvData;
}


CCliTcpServer* CCliTcpServer::_instance = NULL;

CCliTcpServer* CCliTcpServer::instance() 
{
	if (NULL == _instance)
	{
		boost::unique_lock<boost::mutex> lock(g_insMutex);
		if (NULL == _instance)
		{
			_instance = new CCliTcpServer;
		}
	}
	return _instance;
}

CCliTcpServer::CCliTcpServer()
{    
    return ;
}

IParsePacket *CCliTcpServer::createParsePacket()
{
    return new CCliParsePacket;
}

IDataWorkManager *CCliTcpServer::createWorkManager()
{
    return new CCliManager(this);
}

int CCliTcpServer::getServerPort()
{    
	CSimpleIniA ini;  
	ini.SetUnicode();  
	ini.LoadFile("Config.ini");
    return (int)ini.GetLongValue("NetConfig", "CliSerPort");
}


CCliUdpServer* CCliUdpServer::_instance = NULL;

CCliUdpServer* CCliUdpServer::instance() 
{
	if (NULL == _instance)
	{
		boost::unique_lock<boost::mutex> lock(g_insMutex);
		if (NULL == _instance)
		{
			_instance = new CCliUdpServer;
		}
	}
	return _instance;
}

CCliUdpServer::CCliUdpServer()
{   trace_worker();
    return ;
}

IParsePacket *CCliUdpServer::createParsePacket()
{   trace_worker();
    return new CCliParsePacket;
}

IDataWorkManager *CCliUdpServer::createWorkManager()
{   trace_worker();
    return new CCliManager(this);
}

int CCliUdpServer::getServerPort()
{   trace_worker();
	CSimpleIniA ini;  
	ini.SetUnicode();  
	ini.LoadFile("Config.ini");
    return (int)ini.GetLongValue("NetConfig", "CliSerPort");
}


