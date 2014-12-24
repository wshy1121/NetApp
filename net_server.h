#ifndef __NET_SERVER_H
#define __NET_SERVER_H
#include "thread_base.h"
#include "link_tool.h"
#include "defs.h"

#include <list>


class CNetServer
{
public:
	static CNetServer* instance();
public:
	bool startServer();
private:
	static void *listenThread(void *arg);
	static void *clientThread(void *arg);	
private:
	CNetServer();
	void *_listenThread(void *arg);
	void *_clientThread(void *arg);	
	int receive(SOCKET fd,char *szText,int len);
	int send(SOCKET fd,char *szText,int len);
private:
	const int SERVER_PORT;
	
	base::pthread_t m_hListenThread;
	base::pthread_t m_hClientThread;
	
	static  CNetServer* _instance;
	SOCKET m_sockLister;

	typedef std::list<SOCKET> SocketList;
	SocketList m_listClientRead;
	SocketList m_listClientWrite;

	CPthreadMutex m_clientReadMutex;
	CPthreadMutex m_clienttWriteMutex;

};

#endif //_CHAT_ROOT_SERVER_

