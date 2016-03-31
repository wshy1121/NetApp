#ifndef __CLI_SERVER_H
#define __CLI_SERVER_H
#include "net_server.h"
#include "data_work.h"

class CCliServer : public INetServer
{
public:
    static CCliServer* instance();
private:    
    CCliServer();
    virtual IParsePacket *createParsePacket();
    virtual IDataWorkManager *createWorkManager();
    virtual int getServerPort();
private:
	static  CCliServer* _instance;

};

class CCliManager : public IDataWorkManager
{
public:
    CCliManager(INetServer* const netServer = NULL);
    virtual void dealitemData(ClientConn *pClientConn, RECV_DATA *pRecvData);
};

class CCliParsePacket : public IParsePacket
{
public:
    CCliParsePacket(){}
    virtual bool parsePacket(char &charData, char **pPacket);
};

#endif

