#include "stdafx.h"
#include "socket_base.h"
#include "defs.h"
#include "net_client.h"
#include "link_tool.h"
#include "safe_server.h"
#include "data_work.h"
#include "verify_handel.h"
#include "SimpleIni.h"
#include "trace_server.h"
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


CNetClient::CNetClient() : m_maxSessionId(1024*1024), m_sessionId(1), m_socketClient(INVALID_SOCKET)
{
	m_dataWorkManager = new CTraceManager;
	return ;
}


bool CNetClient::connect(char *sip)
{
	m_socketClient = socket(AF_INET, SOCK_STREAM, 0);
	if(INVALID_SOCKET == m_socketClient)
	{
		return false;
	}

	CSimpleIniA ini;  
	ini.SetUnicode();  
	ini.LoadFile("Config.ini");  
	int serverPort = (int)ini.GetLongValue("NetConfig", "NetSerPort");

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
int CNetClient::dealPacket(char *packet, int packetLen, CLogDataInf &dataInf)
{
	char sendData[16];
	int sendDataLen = 0;
	int strLenNum = packetLen + 12;

	memcpy(sendData+sendDataLen, (char *)"\x7B\x7B\x7B\x7B", 4);
	sendDataLen += 4;
	memcpy(sendData+sendDataLen, &strLenNum, sizeof(int));
	sendDataLen += sizeof(int);

	CGuardMutex guardMutex(socketMutex);	
	send(sendData, sendDataLen);
	send(packet, packetLen);
	send((char *)"\x7D\x7D\x7D\x7D", 4);
	receiveInfData(&dataInf);	
	return 0;
}

int CNetClient::send(char *szText,int len)
{
	return m_dataWorkManager->send(m_socketClient, szText, len);
}

bool CNetClient::receiveInfData(CLogDataInf *pDataInf)
{
	char *packet = NULL;
    IParsePacket parsePacket;
	bool bRet = m_dataWorkManager->receiveInfData(m_socketClient, &parsePacket, &packet);
	pDataInf->unPacket(packet);
	return bRet;
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

bool CNetClient::getClientInf(IClientInf *clientInf)
{	trace_worker();
	return CVerifyClient::instance()->getClientInf(clientInf);
}

bool CNetClient::cleanFile(const char *fileName)
{	trace_worker();
	return CTraceClient::instance()->cleanFile(fileName);
}

bool CNetClient::getTraceFileList(CTraceClient::TraceFileVec &fileList)
{
	return CTraceClient::instance()->getTraceFileList(fileList);
}


bool CNetClient::getTraceFileInf(const char *fileName, TraceFileInf &traceFileInf)
{
	return CTraceClient::instance()->getTraceFileInf(fileName, traceFileInf);
}

