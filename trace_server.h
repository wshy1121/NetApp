#ifndef __TRACE_SERVER_H
#define __TRACE_SERVER_H
#include "net_server.h"
#include "data_work.h"

class CTraceServer : public ITcpServer
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
    virtual void dealitemData(RECV_DATA *pRecvData);    
	virtual void dealSendData(TimeCalcInf *pCalcInf);
	virtual void openFile(ClientConn clientConn, char *fileName);
	virtual void closeFile(ClientConn clientConn);
    virtual void dealException(ClientConn clientConn);
};

class CTraceParsePacket : public IParsePacket
{
public:
    CTraceParsePacket();
    virtual bool parsePacket(char &charData, std::string &packet);
private:    
	void initPacketInf();
private:    
	unsigned int m_headCount;
	unsigned int m_tailCount;	
	unsigned int m_curPacketSize;
};

#endif

