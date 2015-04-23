#ifndef __NET_SERVER_H
#define __NET_SERVER_H
#include "thread_base.h"
#include "link_tool.h"
#include "defs.h"
#include <bitset>
#include <memory>
#include "link_tool.h"
#include "data_handle.h"
#include "user_manager.h"

#define MAX_CLIENT_SIZE  1024*1024

typedef struct ClientConn
{
	SOCKET socket;
	int clientId;
	std::shared_ptr<CClientInf> clientInf;
	struct node node;
}ClientConn;
#define clientConnContain(ptr)  container_of(ptr, ClientConn, node)


class CNetServer
{
public:
	static CNetServer* instance();
public:
	bool startServer();
	void pushRecvData(RECV_DATA *pRecvData);	
	node *dealDisconnect(ClientConn *pClientConnRead);
private:
	static void *listenThread(void *arg);
private:
	CNetServer();
	void *_listenThread(void *arg);
	ClientConn *dealConnect(int clientId);
	int creatClientId();
	void setClientId(int clientId);
	void resetClientId(int clientId);
	void sendThreadProc();
	void dealRecvData(TimeCalcInf *pCalcInf);
	void setNoBlock(int socket);
private:
	int SERVER_PORT;
	base::pthread_t m_hListenThread;
	
	static  CNetServer* _instance;
	SOCKET m_sockLister;

	base::CList *m_listClientRead;
	base::CList *m_recvList;
	base::CPthreadMutex m_recvListMutex;
	std::bitset<MAX_CLIENT_SIZE> m_cientIds;
	int m_newId;
	SOCKET m_nfds;
};

#endif //_CHAT_ROOT_SERVER_

