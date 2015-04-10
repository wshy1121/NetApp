#include "stdafx.h"
#include "socket_base.h"
#include "defs.h"
#include "net_client.h"
#include "link_tool.h"
#include "safe_server.h"
#include "data_work.h"
#include "verify_handel.h"
#include <sys/types.h>

using namespace base;
extern CPthreadMutex g_insMutexCalc;
CNetClient* CNetClient::_instance = NULL;

CNetClient* CNetClient::instance() 
{	
	if (NULL == _instance)
	{
		CGuardMutex guardMutex(g_insMutexCalc);
		if (NULL == _instance)
		{
			_instance = new CNetClient;
		}
	}
	return _instance;
}


CNetClient::CNetClient() : m_socketClient(INVALID_SOCKET), m_sessionId(1), m_maxSessionId(1024*1024)
{
	return ;
}


bool CNetClient::connect(char *sip)
{
	m_socketClient = socket(AF_INET, SOCK_STREAM, 0);
	if(INVALID_SOCKET == m_socketClient)
	{
		return false;
	}

	struct sockaddr_in addr;
	addr.sin_family 		= AF_INET;
	addr.sin_addr.s_addr	= inet_addr(sip);
	addr.sin_port			= htons(8889);
	
	int ret = ::connect(m_socketClient, (struct sockaddr *) & addr, sizeof(sockaddr_in));
	if(SOCKET_ERROR == ret)
	{
		return false;
	}

	m_sip = sip;
	return true;
}

bool CNetClient::disConnect()
{
	base::close(m_socketClient);
	return true;
}

int CNetClient::send(char *szText,int len)
{
	return CDataWorkManager::instance()->send(m_socketClient, szText, len);
}

bool CNetClient::receiveInfData(base::CLogDataInf *pDataInf)
{
	return CDataWorkManager::instance()->receiveInfData(m_socketClient, pDataInf);
}

int CNetClient::getSessionId()
{
	if (m_sessionId > m_maxSessionId)
	{
		m_sessionId = 0;
		
	}
	return ++m_sessionId;
}
bool CNetClient::verify(char *userName, char *passWord)
{
	return CVerifyClient::instance()->verify(userName, passWord);
}


