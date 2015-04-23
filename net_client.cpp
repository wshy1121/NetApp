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
	int serverPort = 880110;
#if defined(_DEBUG)
	serverPort = 8889;
#endif

	struct sockaddr_in addr;
	addr.sin_family 		= AF_INET;
	addr.sin_addr.s_addr	= inet_addr(sip);
	addr.sin_port			= htons(serverPort);
	
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
bool CNetClient::login(char *userName, char *passWord)
{	trace_worker();
	return CVerifyClient::instance()->login(userName, passWord);
}


bool CNetClient::createAccess(char *access, int &accessLen)
{	trace_worker();
	return CVerifyClient::instance()->createAccess(access, accessLen);
}

bool CNetClient::getAccessRep(char *access, int accessLen, char *accessRep)
{	trace_worker();
	return CVerifyClient::instance()->getAccessRep(access, accessLen, accessRep);
}


bool CNetClient::verifyAccess(char *access, int accessLen, char *accessRep)
{	trace_worker();
	return CVerifyClient::instance()->verifyAccess(access, accessLen, accessRep);
}

bool CNetClient::getClientInf(CClientInf *clientInf)
{	trace_worker();
	return CVerifyClient::instance()->getClientInf(clientInf);
}

bool CNetClient::getTraceFileList(CTraceClient::StrVec &fileList)
{
	return CTraceClient::instance()->getTraceFileList(fileList);
}

