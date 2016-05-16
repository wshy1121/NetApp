#include <strings.h>
#include "udp_client.h"

CUdpClient::CUdpClient()
:m_serverIp("127.0.0.1")
,m_serverPort(-1)
,m_clientSocket(-1)
{
}

CUdpClient::~CUdpClient()
{
}

int CUdpClient::setSerInf(std::string serverIp, int serverPort)
{
    m_serverIp = serverIp;
    m_serverPort = serverPort;

    if ((m_clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)   
    {
        printf("CUdpClient::setSerInf  socket failed\n");
        return -1;  
    }

    const int opt = 1;  
    //设置该套接字为广播类型，  
    int nb = 0;  
    nb = setsockopt(m_clientSocket, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt));  
    if(nb == -1)  
    {  
        printf("setsockopt  SO_BROADCAST failed\n");
        return -1;  
    }

    bzero(&m_clientAddr, sizeof(struct sockaddr_in));  
    m_clientAddr.sin_family=AF_INET;  
    m_clientAddr.sin_addr.s_addr = inet_addr(serverIp.c_str());  
    m_clientAddr.sin_port=htons(serverPort);  
    return 0;
}


int CUdpClient::send(const char *sendData, int sendDataLen)
{
    return sendto(m_clientSocket, sendData, sendDataLen, 0, (struct sockaddr *)&m_clientAddr, sizeof(m_clientAddr));
}


