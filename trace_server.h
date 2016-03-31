#ifndef __TRACE_SERVER_H
#define __TRACE_SERVER_H
#include "net_server.h"
#include "data_work.h"

class CTraceServer : public INetServer
{
public:
    static CTraceServer* instance();
private:    
    CTraceServer();
    virtual IParsePacket *createParsePacket();
    virtual IDataWorkManager *createWorkManager();
    virtual int getServerPort();
private:
	static  CTraceServer* _instance;

};

class CTraceManager : public IDataWorkManager
{
public:
    CTraceManager(INetServer* const netServer = NULL);
    virtual void dealitemData(ClientConn *pClientConn, RECV_DATA *pRecvData);
};

class CTraceParsePacket : public IParsePacket
{
public:
    CTraceParsePacket(){}
    virtual bool parsePacket(char &charData, char **pPacket);
};

#endif

