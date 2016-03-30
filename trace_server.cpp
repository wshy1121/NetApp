#include <boost/thread/mutex.hpp>
#include "trace_server.h"
#include "link_tool.h"
#include "verify_handel.h"
#include "trace_handel.h"
#include "SimpleIni.h"

static boost::mutex g_insMutex;

CTraceServer* CTraceServer::_instance = NULL;

CTraceServer* CTraceServer::instance() 
{
	if (NULL == _instance)
	{
		boost::unique_lock<boost::mutex> lock(g_insMutex);
		if (NULL == _instance)
		{
#ifdef WIN32	
			WSADATA wsa={0};
			WSAStartup(MAKEWORD(2,2),&wsa);
#endif
			_instance = new CTraceServer;
		}
	}
	return _instance;
}

CTraceServer::CTraceServer()
{
	new CVerifyHandle;
	new CTraceHandle;

	CSimpleIniA ini;  
	ini.SetUnicode();  
	ini.LoadFile("Config.ini");
    
	SERVER_PORT = (int)ini.GetLongValue("NetConfig", "NetSerPort");
	m_dataWorkManager = new CTraceManager(this);
    return ;
}

CTraceManager::CTraceManager(CTraceServer* const netServer)
{
    m_netServer = netServer;
}



