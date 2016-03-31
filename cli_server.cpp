#include <boost/thread/mutex.hpp>
#include "cli_server.h"
#include "link_tool.h"
#include "verify_handel.h"
#include "trace_handel.h"
#include "SimpleIni.h"

extern FILE *rl_instream;
extern FILE *rl_outstream;

static boost::mutex g_insMutex;

CCliServer* CCliServer::_instance = NULL;

CCliServer* CCliServer::instance() 
{
	if (NULL == _instance)
	{
		boost::unique_lock<boost::mutex> lock(g_insMutex);
		if (NULL == _instance)
		{
			_instance = new CCliServer;
		}
	}
	return _instance;
}

CCliServer::CCliServer()
{    
    return ;
}

IParsePacket *CCliServer::createParsePacket()
{
    return new CCliParsePacket;
}

IDataWorkManager *CCliServer::createWorkManager()
{
    return new CCliManager(this);
}

int CCliServer::getServerPort()
{    
	CSimpleIniA ini;  
	ini.SetUnicode();  
	ini.LoadFile("Config.ini");
    return (int)ini.GetLongValue("NetConfig", "CliSerPort");
}



CCliManager::CCliManager(INetServer* const netServer)
{
    m_netServer = netServer;
}

void CCliManager::dealException(ClientConn clientConn)
{
    printf("CCliServer client disconnect socket:%d clientId:%d\n", clientConn.socket, clientConn.clientId);
}

void CCliManager::dealitemData(ClientConn *pClientConn, RECV_DATA *pRecvData)
{
    IDealDataHandle::destroyRecvData(pRecvData);
}

bool CCliParsePacket::parsePacket(char &charData, char **pPacket)
{   trace_worker();
    trace_printf("charData  %c|", charData);
    char &posData = m_packetBuffer[m_packetPos];
    posData = charData;
    *pPacket = &posData;
    m_packetPos = ++m_packetPos % m_maxBufferSize;
    printf("%d ", m_packetPos);
    return true;
}

