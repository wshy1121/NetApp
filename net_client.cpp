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


CNetClient::CNetClient() : m_socketClient(INVALID_SOCKET)
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

bool CNetClient::verify(char *userName, char *passWord)
{
	CLogDataInf dataInf;
	dataInf.putInf((char *)"verify");
	dataInf.putInf("1");//session id(必须大于0)
	dataInf.putInf("0");//主密钥索引(16字节)
	dataInf.putInf("0");//实际密钥索引(16字节)
	dataInf.putInf(""); //密钥索引映射表(256字节)
	dataInf.putInf("");
	dataInf.putInf("0");
	dataInf.putInf("");

	char *packet = NULL;
	int packetLen = dataInf.packet(packet);
	CDataWorkManager::instance()->send(m_socketClient, packet, packetLen);
	CDataWorkManager::instance()->receiveInfData(m_socketClient, &dataInf);	

	CSafeServer::uchar keyInf[KEY_INF_LEN];
	CSafeServer::instance()->createKeyInf(keyInf, sizeof(keyInf));
	CSafeServer::instance()->encode(keyInf, sizeof(keyInf), NULL, 0, NULL);
	return true;
}


