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
    return new CTraceParsePacket;
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


bool CTraceParsePacket::parsePacket(char &charData, char **pPacket)
{
	if (m_curPacketSize == 0 && m_packetPos > 8)
	{
		memcpy(&m_curPacketSize, m_packetBuffer+4, 4);
	}
	
	if (m_curPacketSize > 0)
	{
		if (m_curPacketSize > m_maxBufferSize || m_packetPos > m_curPacketSize)
		{
			initPacketInf();
		}
	}
	
	switch (charData)
	{
		case '\x7B':
            m_tailCount = 0;
			++m_headCount;
			++m_packetPos;
			break;
		case '\x7D':
            if (m_headCount < 4)
			{
				initPacketInf();
			}
			else
            {
                ++m_tailCount;
            }
            
			++m_packetPos;
			if (m_tailCount >= 4)
			{
					if (m_curPacketSize == m_packetPos)
				{
					char *packet = new char[m_packetPos];
					memcpy(packet, m_packetBuffer + 8, m_packetPos - 12);
					*pPacket = packet;
					initPacketInf();
					return true;
				}
			}
			break;
		default:
            m_tailCount = 0;
			if (m_headCount < 4)
			{
				initPacketInf();
			}				
			++m_packetPos;
			break;
	}
	return false;
}

