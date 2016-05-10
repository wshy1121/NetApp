#ifndef _UDP_SERVER_H_
#define _UDP_SERVER_H_
#include "net_server.h"


class IUdpServer:public INetServer
{
public:
	IUdpServer();
    ~IUdpServer();
};


#endif

