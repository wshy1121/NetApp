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
private:	
	static  CNetClient* _instance;
	SOCKET m_socketClient;
	std::string m_sip;
};



#endif

