#ifndef _UDP_SERVER_H_
#define _UDP_SERVER_H_
#include "net_server.h"

class IUdpServer:public INetServer
{
public:
    typedef boost::shared_ptr<boost::thread> WorkThread;    
	IUdpServer();
    virtual ~IUdpServer();
	virtual bool startServer();
    virtual int send(IClientInf *clientInf, char *szText,int len);
private:
	void listenThread();
    bool receiveInfData(int socket, IParsePacket *parsePacket, std::string &packet);
    void initClientConn(ClientConn &clientConn);
private:
    const int m_maxBufferSize;
};


#endif

