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
    virtual void dealitemData(RECV_DATA *pRecvData);
    virtual void dealException(ClientConn clientConn);
};

class CCliParsePacket : public IParsePacket
{
public:    
    typedef boost::shared_ptr<boost::thread> DataWorkThread;
    CCliParsePacket();
    void initPacketInf();
    virtual bool parsePacket(char &charData, std::string &packet);
    virtual void writeData(char *data, int dataLen);
private:
    void cliThreadProc();
    void sendThreadProc();
private:
    DataWorkThread m_cliThread;
    DataWorkThread m_sendThread;
    
    int m_instream[2];
    int m_outstream[2];
    FILE *m_instreamFile;
    FILE *m_outstreamFile;
};

#endif

