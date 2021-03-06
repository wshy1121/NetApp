#ifndef __DATA_WORK_H
#define __DATA_WORK_H

#include "link_tool.h"
#include "time_calc.h"
#include "net_server.h"

typedef struct WORK_DATA
{
	char *m_pContent;
	int m_contentLen;
	int clientId;
	struct node node;
}WORK_DATA;

#define workDataContain(ptr)  container_of(ptr, WORK_DATA, node)


class IDataWorkManager
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
    typedef boost::shared_ptr<boost::thread> WorkThread;
public:    
	IDataWorkManager();
	virtual ~IDataWorkManager();
	void pushItemData(ClientConn *pClientConn, RECV_DATA *pRecvData);	
	virtual void dealitemData(RECV_DATA *pRecvData) = 0;	
	void dealErrNo(ClientConn *pClientConnRead);
	void dealDisConnect(ClientConn *pClientConnRead);
	bool receiveInfData(int socket, IParsePacket *parsePacket, std::string &packet);	
	int receive(int fd,char *szText,int len);
	int send(int fd,char *szText,int len);	
	virtual void openFile(ClientConn clientConn, char *fileName);
	virtual void closeFile(ClientConn clientConn);
	virtual void dealSendData(TimeCalcInf *pCalcInf) = 0;
private:
	void threadProc();
	void setErrNo(int recvNum);	
	virtual void dealException(ClientConn clientConn);
protected:
	ErrNo m_errNo;	
	base::CList *m_recvList;
	base::CPthreadMutex m_recvListMutex;
	WorkThread m_threadId;

	unsigned int m_headCount;
	unsigned int m_tailCount;	
	unsigned int m_packetPos;
	char *m_packetBuffer;
	unsigned int m_curPacketSize;
	const unsigned int m_maxBufferSize;
    INetServer *m_netServer;
};



#endif

