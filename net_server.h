#ifndef __NET_SERVER_H
#define __NET_SERVER_H
#include "thread_base.h"
#include "link_tool.h"
#include "defs.h"
#include <bitset>

#define MAX_CLIENT_SIZE  1024*1024

typedef struct ClientConn
{
	SOCKET socket;
	int clientId;
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
	void dealException(int clientId);
	node *dealDisconnect(ClientConn *pClientConnRead);
	ClientConn *dealConnect(int clientId);
	void openFile(int fileKey, char *fileName);
	void closeFile(int fileKey);
	bool receiveInfData(int socket, char *infs[], int infLens[]);
	int receive(SOCKET fd,char *szText,int len);
	int send(SOCKET fd,char *szText,int len);
	int creatClientId();
	void setClientId(int clientId);
	void resetClientId(int clientId);
private:
	const int SERVER_PORT;
	
	base::pthread_t m_hListenThread;
	
	static  CNetServer* _instance;
	SOCKET m_sockLister;

	base::CList *m_listClientRead;	
	char *m_recvBuf;
	int m_recvBufLen;
	std::bitset<MAX_CLIENT_SIZE> m_cientIds;
	int m_newId;
	int m_nfds;
};

#endif //_CHAT_ROOT_SERVER_

