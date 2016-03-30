#ifndef __NET_SERVER_H
#define __NET_SERVER_H
#include "link_tool.h"
#include "defs.h"
#include <bitset>
#include <memory>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

#include "link_tool.h"
#include "data_handle.h"
#include "user_manager.h"

class IDataWorkManager;

typedef struct ClientConn
{
	int socket;
	int clientId;
	std::shared_ptr<CClientInf> clientInf;
	struct node node;
}ClientConn;
#define clientConnContain(ptr)  container_of(ptr, ClientConn, node)


class INetServer
{
public:
    typedef boost::shared_ptr<boost::thread> WorkThread;    
	INetServer();
    virtual ~INetServer();
	bool startServer();
	void pushRecvData(RECV_DATA *pRecvData);	
	node *dealDisconnect(ClientConn *pClientConnRead);
private:
	void listenThread();
	void sendThreadProc();
private:
	ClientConn *dealConnect(int socket, sockaddr_in &clientAddr);
	int creatClientId();
	void dealRecvData(TimeCalcInf *pCalcInf);
	void setNoBlock(int socket);
protected:
	IDataWorkManager *m_dataWorkManager;
	int SERVER_PORT;
	WorkThread  m_hListenThread;
	WorkThread m_sendThread;

	
	int m_sockLister;

	base::CList *m_listClientRead;
	base::CList *m_recvList;
	base::CPthreadMutex m_recvListMutex;
	int m_newId;
	int m_nfds;
};


#endif //_CHAT_ROOT_SERVER_

