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
private:
	static  CTraceServer* _instance;

};

class CTraceManager : public IDataWorkManager
{
public:
    CTraceManager(CTraceServer* const netServer = NULL);
};

class CClientInf : public IClientInf
{
public:
    CClientInf();
};



#endif

