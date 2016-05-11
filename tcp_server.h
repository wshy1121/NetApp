#ifndef __TCP_SERVER_H
#define __TCP_SERVER_H
#include "net_server.h"

class ITcpServer : public INetServer
{
public:
    ITcpServer(){}
    virtual ~ITcpServer(){}
public:
    virtual bool startServer();
    virtual int send(IClientInf *clientInf, char *szText,int len);
private:
    void listenThread();
    bool receiveInfData(int socket, IParsePacket *parsePacket, std::string &packet);
};


#endif

