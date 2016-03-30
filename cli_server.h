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
private:
	static  CCliServer* _instance;

};

class CCliManager : public IDataWorkManager
{
public:
    CCliManager(CCliServer* const netServer = NULL);
};


#endif

