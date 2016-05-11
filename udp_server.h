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
private:
	void listenThread();
    bool receiveInfData(int socket, IParsePacket *parsePacket, std::string &packet);
};


#endif

