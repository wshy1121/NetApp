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

CTraceManager::CTraceManager(INetServer* const netServer)
{
    m_netServer = netServer;
}

void CTraceManager::dealitemData(RECV_DATA *pRecvData)
{
	TimeCalcInf *pCalcInf = &pRecvData->calcInf;

	if (pCalcInf->m_packet.size())
	{
		pCalcInf->m_dataInf->unPacket((char *)pCalcInf->m_packet.c_str());
	}
	IDealDataHandle::execute(pCalcInf);
}

void CTraceManager::openFile(ClientConn clientConn, char *fileName)
{
	RECV_DATA *pRecvData =IDealDataHandle::createRecvData();
	
	std::shared_ptr<CLogDataInf> &dataInf = pRecvData->calcInf.m_dataInf;
	
	dataInf->putInf("openFile");
	dataInf->putInf("1");//session id
	dataInf->putInf("0");
	dataInf->putInf("0");
	dataInf->putInf("");
	dataInf->putInf("");
	dataInf->putInf("0");
	dataInf->putInf(fileName);

	clientConn.socket = INVALID_SOCKET;
	pushItemData(&clientConn, pRecvData); 
	return ;
}


void CTraceManager::closeFile(ClientConn clientConn)
{
	RECV_DATA *pRecvData = IDealDataHandle::createRecvData();
	
	std::shared_ptr<CLogDataInf> &dataInf = pRecvData->calcInf.m_dataInf;
	dataInf->putInf("closeFile");	
	dataInf->putInf("1");//session id
	dataInf->putInf("0");
	dataInf->putInf("0");
	dataInf->putInf("");
	dataInf->putInf("");
	dataInf->putInf("0");
	dataInf->putInf("");

	clientConn.socket = INVALID_SOCKET;
	pushItemData(&clientConn, pRecvData); 
	return ;
}


void CTraceManager::dealException(ClientConn clientConn)
{
	clientConn.socket = INVALID_SOCKET;
	{
		RECV_DATA *pRecvData = IDealDataHandle::createRecvData();

		std::shared_ptr<CLogDataInf> &dataInf = pRecvData->calcInf.m_dataInf;
		dataInf->putInf("dispAll");		
		dataInf->putInf("1");//session id
		dataInf->putInf("0");
		dataInf->putInf("0");
		dataInf->putInf("");
		dataInf->putInf("");
		dataInf->putInf("0");
		dataInf->putInf("backtrace");

		pushItemData(&clientConn, pRecvData); 
	}
	{
		RECV_DATA *pRecvData = IDealDataHandle::createRecvData();

		std::shared_ptr<CLogDataInf> &dataInf = pRecvData->calcInf.m_dataInf;
		dataInf->putInf("cleanAll");		
		dataInf->putInf("1");//session id
		dataInf->putInf("0");
		dataInf->putInf("0");
		dataInf->putInf("");
		dataInf->putInf("");
		dataInf->putInf("0");
		dataInf->putInf("backtrace");

		pushItemData(&clientConn, pRecvData); 
	}
	return ;
}

CTraceParsePacket::CTraceParsePacket()
:m_headCount(0)
,m_tailCount(0)
,m_curPacketSize(0)
{
}


void CTraceParsePacket::initPacketInf()
{
    m_headCount = 0;
    m_tailCount = 0;
    m_packetPos = 0;
    m_curPacketSize = 0;
}

bool CTraceParsePacket::parsePacket(char &charData, std::string &packet)
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
                    packet.assign(m_packetBuffer + 8, m_packetPos - 12);
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

