#ifndef _UDP_CLIENT_H_
#define _UDP_CLIENT_H_
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class CUdpClient
{
public:
    CUdpClient();
    virtual ~CUdpClient();
public:
    int setSerInf(std::string serverIp, int serverPort);
    int send(const char *sendData, int sendDataLen);
protected:
    std::string m_serverIp;
    int m_serverPort;
    int m_clientSocket;
    sockaddr_in m_clientAddr;
};

#endif

