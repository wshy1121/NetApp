#ifndef __NET_SERVER_H
#define __NET_SERVER_H
#include "thread_base.h"
#include "link_tool.h"
#include "defs.h"

typedef struct ClientConn
{
	SOCKET socket;
	struct node node;
}ClientConn;
#define clientConnContain(ptr)  container_of(ptr, ClientConn, node)


class CNetServer
{
public:
	static CNetServer* instance();
public:
	bool startServer();
private:
	static void *listenThread(void *arg);
private:
	CNetServer();
	void *_listenThread(void *arg);
	int receive(SOCKET fd,char *szText,int len);
	int send(SOCKET fd,char *szText,int len);
private:
	const int SERVER_PORT;
	
	base::pthread_t m_hListenThread;
	
	static  CNetServer* _instance;
	SOCKET m_sockLister;

	CList *m_listClientRead;
};

#endif //_CHAT_ROOT_SERVER_

