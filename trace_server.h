#ifndef __TRACE_SERVER_H
#define __TRACE_SERVER_H
#include "net_server.h"
#include "data_work.h"

class CNetServer : public INetServer
{
public:
    static CNetServer* instance();
private:    
    CNetServer();
private:
	static  CNetServer* _instance;

};

class CDataWorkManager : public IDataWorkManager
{
public:
    CDataWorkManager(CNetServer* const netServer = NULL);
};


#endif

