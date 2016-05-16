#ifndef __CLI_CLIENT_H
#define __CLI_CLIENT_H
#include "udp_client.h"
#include "Json/json.h"

class CCliUdpClient : public CUdpClient
{
public:
    static CCliUdpClient* instance();
public:
    bool getSerIp(Json::Value serverIps);
private:    
    CCliUdpClient();
private:
    static  CCliUdpClient* _instance;
};
#endif

