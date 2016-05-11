#ifndef __NET_SERVER_H
#define __NET_SERVER_H

#include <bitset>
#include <memory>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

#include "defs.h"
#include "link_tool.h"
#include "data_handle.h"
#include "user_manager.h"

class IDataWorkManager;

typedef struct ClientConn
{
	int socket;
	int clientId;
	std::shared_ptr<IClientInf> clientInf;
	struct node node;
}ClientConn;
#define clientConnContain(ptr)  container_of(ptr, ClientConn, node)


class INetServer
{
public:
    friend class ITcpServer;
    friend class IUdpServer;
    typedef boost::shared_ptr<boost::thread> WorkThread;    
	INetServer();
    virtual ~INetServer();
	virtual bool startServer() = 0;
    virtual int send(IClientInf *clientInf, char *szText,int len) = 0;
    
	void pushRecvData(RECV_DATA *pRecvData);	
	node *dealDisconnect(ClientConn *pClientConnRead);
private:
	void sendThreadProc();
protected:
	ClientConn *dealConnect(int socket, sockaddr_in &clientAddr);
    IClientInf *createClientInf();
    virtual IParsePacket *createParsePacket() = 0;
    int creatClientId();
	void setNoBlock(int socket);     
    virtual int getServerPort() = 0;
    virtual IDataWorkManager *createWorkManager() = 0;
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


#endif 

