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

typedef struct ClientConn
{
	int socket;
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
	static void *sendThreadProc(void *arg);
private:
	CNetServer();
	void *_listenThread(void *arg);
	ClientConn *dealConnect(int socket);
	int creatClientId();
	void *_sendThreadProc(void *arg);
	void dealRecvData(TimeCalcInf *pCalcInf);
	void setNoBlock(int socket);
private:
	CDataWorkManager *m_dataWorkManager;
	int SERVER_PORT;
	base::pthread_t m_hListenThread;
	base::pthread_t m_sendThread;

	
	static  CNetServer* _instance;
	int m_sockLister;

	base::CList *m_listClientRead;
	base::CList *m_recvList;
	base::CPthreadMutex m_recvListMutex;
	int m_newId;
	int m_nfds;
};

#endif //_CHAT_ROOT_SERVER_

