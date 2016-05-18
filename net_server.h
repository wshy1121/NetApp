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
#include "event.h"

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
    virtual bool doRead(ClientConn *clientConn) = 0;
    
    static bool initEvent();
    static int dispatch();
    static void doRead(evutil_socket_t clientSocket, short events, void *arg);
	void pushRecvData(RECV_DATA *pRecvData);	
	void dealDisconnect(ClientConn *pClientConnRead);
private:
	void sendThreadProc();
protected:
    ClientConn *createClientConn(int socket, sockaddr_in *clientAddr);
    void destroyClientConn(ClientConn *pClientConnRead);
	ClientConn *dealConnect(int socket, sockaddr_in *clientAddr);
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

	
	evutil_socket_t m_sockLister;

	base::CList *m_recvList;
	base::CPthreadMutex m_recvListMutex;
	static int m_newId;

    static struct event_base *m_base;
};


#endif 

