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
    typedef boost::shared_ptr<boost::thread> WorkThread;    
	INetServer();
    virtual ~INetServer();
	virtual bool startServer() = 0;
	void pushRecvData(RECV_DATA *pRecvData);	
	node *dealDisconnect(ClientConn *pClientConnRead);
private:
	void sendThreadProc();
private:
	ClientConn *dealConnect(int socket, sockaddr_in &clientAddr);
    IClientInf *createClientInf();
    virtual IParsePacket *createParsePacket() = 0;
	bool receiveInfData(int socket, IParsePacket *parsePacket, std::string &packet);
    int creatClientId();
	void setNoBlock(int socket);    
protected:    
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


#endif //_CHAT_ROOT_SERVER_

