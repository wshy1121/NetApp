#include "stdafx.h"
#include "socket_base.h"
#include "defs.h"
#include "net_client.h"
#include "link_tool.h"
#include "safe_server.h"
#include "data_work.h"
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
	char sessionId[16];
	snprintf(sessionId, sizeof(sessionId), "%d", getSessionId());
	
	char keyInf[KEY_INF_LEN];
	CSafeServer::instance()->createKeyInf(keyInf, sizeof(keyInf));

	char _userName[32];
	int _userNameLen = sizeof(_userName);
	CSafeServer::instance()->encode(keyInf, sizeof(keyInf), userName, strlen(userName)+1, _userName, _userNameLen);

	char _passWord[32];
	int _passWordLen = sizeof(_passWord);
	CSafeServer::instance()->encode(keyInf, sizeof(keyInf), passWord, strlen(passWord)+1, _passWord, _passWordLen);

	CLogDataInf dataInf;

	dataInf.putInf((char *)"verify");
	dataInf.putInf(sessionId);//session id(大于0)
	dataInf.putInf(keyInf, sizeof(keyInf));//密钥
	dataInf.putInf(_userName, _userNameLen);//用户名
	dataInf.putInf(_passWord, _passWordLen); //密码

	char *packet = NULL;
	int packetLen = dataInf.packet(packet);
	CDataWorkManager::instance()->send(m_socketClient, packet, packetLen);
	CDataWorkManager::instance()->receiveInfData(m_socketClient, &dataInf);

	{
		char *oper = dataInf.m_infs[0];
		char *sessionId = dataInf.m_infs[1];
		
		char *keyInf = dataInf.m_infs[2];
		int keyInfLen = dataInf.m_infLens[2];
		
		char *userName = dataInf.m_infs[3]; 
		int userNameLen = dataInf.m_infLens[3];
		
		char *passWord = dataInf.m_infs[4];
		int passWordLen = dataInf.m_infLens[4];
		
		
		CSafeServer::instance()->decode(keyInf, keyInfLen, userName, userNameLen,userName);
		CSafeServer::instance()->decode(keyInf, keyInfLen, passWord, passWordLen,passWord);
		printf("CNetClient::verify %s	%s	%s\n", oper, userName, passWord);

	}
	return true;
}


