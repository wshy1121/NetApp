#ifndef __CLI_SERVER_H
#define __CLI_SERVER_H
#include "data_work.h"
#include "tcp_server.h"
#include "udp_server.h"

class CCliManager : public IDataWorkManager
{
public:
    CCliManager(INetServer* const netServer = NULL);
    virtual void dealitemData(RECV_DATA *pRecvData);
    virtual void dealSendData(TimeCalcInf *pCalcInf);
    virtual void dealException(ClientConn clientConn);
};

class CCliParsePacket : public IParsePacket
{
public:    
    typedef boost::shared_ptr<boost::thread> DataWorkThread;
    CCliParsePacket();
    ~CCliParsePacket();
    void initPacketInf();
    virtual bool parsePacket(char *charData, int charDataLen, std::string &packet);
    virtual void writeData(char *data, int dataLen);
private:
    void cliThreadProc();
    void sendThreadProc();
    RECV_DATA *packetRecvData(char *data);
private:
    DataWorkThread m_cliThread;
    DataWorkThread m_sendThread;
    
    int m_instream[2];
    int m_outstream[2];
    FILE *m_instreamFile;
    FILE *m_outstreamFile;
};


class CCliTcpServer : public ITcpServer
{
public:
    static CCliTcpServer* instance();
private:    
    CCliTcpServer();
    virtual IParsePacket *createParsePacket();
    virtual IDataWorkManager *createWorkManager();
    virtual int getServerPort();
private:
	static  CCliTcpServer* _instance;

};


class CCliUdpServer : public IUdpServer
{
public:
    static CCliUdpServer* instance();
private:    
    CCliUdpServer();
    virtual IParsePacket *createParsePacket();
    virtual IDataWorkManager *createWorkManager();
    virtual int getServerPort();
private:
	static  CCliUdpServer* _instance;

};



#endif

