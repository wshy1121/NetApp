#ifndef __NET_CLIENT_H
#define __NET_CLIENT_H
#include "defs.h"
#include "link_tool.h"
#include <string>

class CNetClient
{
public:
	static CNetClient* instance();
	bool disConnect();	
	bool connect(char *sip);	
	int send(char *szText,int len);
	bool receiveInfData(base::CLogDataInf *pDataInf);
public:
	bool login(char *userName, char *passWord);	
	int getSessionId();
private:	
	CNetClient();	
private:	
	static  CNetClient* _instance;
	const int m_maxSessionId;
	int m_sessionId;
	SOCKET m_socketClient;
	std::string m_sip;
};



#endif

