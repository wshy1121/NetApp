#include <boost/thread/mutex.hpp>
#include "cli_server.h"
#include "link_tool.h"
#include "verify_handel.h"
#include "trace_handel.h"
#include "SimpleIni.h"

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



CCliManager::CCliManager(CCliServer* const netServer)
{
    m_netServer = netServer;
}


bool CCliParsePacket::parsePacket(char &charData, char **pPacket)
{

}

