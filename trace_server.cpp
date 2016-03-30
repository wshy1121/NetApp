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
			_instance = new CTraceServer;
		}
	}
	return _instance;
}

CTraceServer::CTraceServer()
{
    new CVerifyHandle;
    new CTraceHandle;
    return ;
}

IParsePacket *CTraceServer::createParsePacket()
{
    return new IParsePacket;
}

IDataWorkManager *CTraceServer::createWorkManager()
{
    return new CTraceManager(this);
}

int CTraceServer::getServerPort()
{
	CSimpleIniA ini;  
	ini.SetUnicode();  
	ini.LoadFile("Config.ini");
	return (int)ini.GetLongValue("NetConfig", "NetSerPort");
}

CTraceManager::CTraceManager(CTraceServer* const netServer)
{
    m_netServer = netServer;
}


