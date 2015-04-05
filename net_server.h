#ifndef __NET_SERVER_H
#define __NET_SERVER_H
#include "thread_base.h"
#include "link_tool.h"
#include "defs.h"
#include <bitset>
#include "link_tool.h"
#include "data_handle.h"

#define MAX_CLIENT_SIZE  1024*1024

typedef struct ClientConn
{
	SOCKET socket;
	int clientId;
	struct node node;
}ClientConn;
#define clientConnContain(ptr)  container_of(ptr, ClientConn, node)


class CNetServer
{
public:
	typedef enum
	{
		e_noErr,
		e_disConnect,	//socket已正常关闭
		e_readOk,		//当前缓冲区已无数据可读
		e_rst,			// 对方发送了RST
		e_intr,			// 被信号中断
	}ErrNo;
	static CNetServer* instance();
public:
	bool startServer();
	void pushRecvData(RECV_DATA *pRecvData);
private:
	static void *listenThread(void *arg);
private:
	CNetServer();
	void *_listenThread(void *arg);
	void dealException(int clientId);
	node *dealDisconnect(ClientConn *pClientConnRead);
	ClientConn *dealConnect(int clientId);
	void openFile(int fileKey, char *fileName);
	void closeFile(int fileKey);
	bool receiveInfData(int socket, base::CLogDataInf *pDataInf);
	int receive(SOCKET fd,char *szText,int len);
	int send(SOCKET fd,char *szText,int len);
	int creatClientId();
	void setClientId(int clientId);
	void resetClientId(int clientId);
	void sendThreadProc();
	void dealRecvData(TimeCalcInf *pCalcInf);
	void setNoBlock(int socket);
	void setErrNo(int recvNum);	
	node *dealErrNo(ClientConn *pClientConnRead, node *pNode);
private:
	const int SERVER_PORT;
	ErrNo m_errNo;
	base::pthread_t m_hListenThread;
	
	static  CNetServer* _instance;
	SOCKET m_sockLister;

	base::CList *m_listClientRead;
	base::CList *m_recvList;
	base::CPthreadMutex m_recvListMutex;
	char *m_recvBuf;
	int m_recvBufLen;
	std::bitset<MAX_CLIENT_SIZE> m_cientIds;
	int m_newId;
	SOCKET m_nfds;
};

#endif //_CHAT_ROOT_SERVER_

