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
    virtual bool doRead(ClientConn *clientConn);
    static void doAccept(evutil_socket_t listenerSocket, short event, void *arg);
private:
    bool receiveInfData(int socket, IParsePacket *parsePacket, std::string &packet);
};


#endif

