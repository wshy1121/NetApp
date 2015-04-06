#include "stdafx.h"
#include "socket_base.h"
#include "defs.h"
#include "net_client.h"
#include "link_tool.h"
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


bool CNetClient::connect(std::string &sip)
{
	m_socketClient = socket(AF_INET, SOCK_STREAM, 0);
	if(INVALID_SOCKET == m_socketClient)
	{
		return false;
	}

	struct sockaddr_in addr;
	addr.sin_family 		= AF_INET;
	addr.sin_addr.s_addr	= inet_addr(m_sip.c_str());
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

int CNetClient::receive(char *szText,int len)
{
	int rc;
	rc=recv(m_socketClient,szText,len,0);
	if(rc <= 0)
	{
		return -1;
	}
	return rc;
}



int CNetClient::send(char *szText,int len)
{
	int cnt;
	int rc;
	cnt=len;
	while(cnt>0)
	{
		rc=::send(m_socketClient,szText,cnt,0);
		if(rc==SOCKET_ERROR)
		{
			return -1;
		}
		if(rc==0)
		{
			return len-cnt;
		}
		szText+=rc;
		cnt-=rc;
	}
	return len;
}

bool CNetClient::verify(char *userName, char *passWord)
{
	CLogDataInf dataInf;
	dataInf.putInf((char *)"insertTrace");
	dataInf.putInf("sTid");
	dataInf.putInf("sLine");
	dataInf.putInf("file_name");
	dataInf.putInf("");
	dataInf.putInf("0");
	dataInf.putInf("content");

	char *packet = NULL;
	int packetLen = dataInf.packet(packet);
	send(packet, packetLen);
	CDataWorkManager::instance()->receiveInfData(m_socketClient, &dataInf);	
	return true;
}


