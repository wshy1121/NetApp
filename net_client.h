#ifndef __NET_CLIENT_H
#define __NET_CLIENT_H
#include "defs.h"
#include <string>

class CNetClient
{
public:
	static CNetClient* instance();
	bool disConnect();	
	bool connect(char *sip);
public:
	bool verify(char *userName, char *passWord);
private:	
	CNetClient();	
	int getSessionId();
private:	
	static  CNetClient* _instance;
	const int m_maxSessionId;
	int m_sessionId;
	SOCKET m_socketClient;
	std::string m_sip;
};



#endif

