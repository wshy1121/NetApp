#ifndef __NET_CLIENT_H
#define __NET_CLIENT_H
#include "defs.h"
#include <string>

class CNetClient
{
public:
	CNetClient(std::string sip);
	bool disConnect();
public:
	bool connect();
	int receive(char *szText,int len);
	int send(char *szText,int len);
private:
	SOCKET m_socketClient;
	std::string m_sip;
};



#endif

