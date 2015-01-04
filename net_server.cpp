#include "stdafx.h"
#include "net_server.h"
#include "string_base.h"
#include "socket_base.h"
#include "data_work.h"
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "open_src.h"
#include "Global.h"



extern CPthreadMutex g_insMutexCalc;
extern char *dataFormat;
CNetServer* CNetServer::_instance = NULL;

CNetServer::CNetServer():SERVER_PORT(8889), m_sockLister(INVALID_SOCKET), m_recvBufLen(1024*1024)
{
	m_recvBuf = (char *)base::malloc(m_recvBufLen);
	m_listClientRead = CList::createCList();
	m_cientIds.reset();
	m_newId = 0;
	return ;
}

CNetServer* CNetServer::instance() 
{	
	if (NULL == _instance)
	{
		CGuardMutex guardMutex(g_insMutexCalc);
		if (NULL == _instance)
		{
#ifdef WIN32	
			WSADATA wsa={0};
			WSAStartup(MAKEWORD(2,2),&wsa);
#endif
			_instance = new CNetServer;
		}
	}
	return _instance;
}

bool CNetServer::startServer()
{
	if(INVALID_SOCKET != m_sockLister)
	{
		printf("server is runing!\n");
		return false;
	}
	
	m_sockLister = socket(AF_INET, SOCK_STREAM, 0);
	if(m_sockLister == INVALID_SOCKET)
	{
		printf("create sock error!\n");
		return false;
	}
	int nRecvBuf = m_recvBufLen;
	setsockopt(m_sockLister, SOL_SOCKET,SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));

	int opt = 0;
	if (setsockopt(m_sockLister, SOL_SOCKET, SO_REUSEADDR, (char*) &opt, sizeof(opt))< 0)
	{
		printf("set sock error!\n");
		return false;
	}
	struct sockaddr_in svraddr;
	svraddr.sin_family = AF_INET;
	svraddr.sin_addr.s_addr = INADDR_ANY;
	svraddr.sin_port = htons(SERVER_PORT);

	int ret = bind(m_sockLister, (struct sockaddr*) &svraddr, sizeof(svraddr));
	if (ret == SOCKET_ERROR)
	{
		printf("bind sock error!\n");
		return false;
	}

	base::pthread_create(&m_hListenThread, NULL,listenThread,NULL);

	printf("server is start!\n");
	return true;
}


void *CNetServer::listenThread(void *arg)
{
	return CNetServer::instance()->_listenThread(arg);
}


void *CNetServer::_listenThread(void *arg)
{
	int backlog = 5;
	int ret = listen(m_sockLister, backlog);
	if (ret == SOCKET_ERROR)
	{
		return NULL;
	}
	fd_set fd_read, fd_write;
	sockaddr_in clientAddr;
	socklen_t nLen = sizeof(sockaddr);
	struct timeval cctv = {0, 50};
	node *pNode = NULL;
	while(true)
	{
		FD_ZERO(&fd_read);
		FD_ZERO(&fd_write);
		FD_SET(m_sockLister, &fd_read);

		ClientConn *pClientConnTmp = NULL;
		each_link_node(&m_listClientRead->head_node, pNode)
		{
			pClientConnTmp = clientConnContain(pNode);
			FD_SET(pClientConnTmp->socket, &fd_read);
		}
		
		int count = select(m_listClientRead->size()+1, &fd_read, &fd_write, NULL, &cctv);
		for (int i=0; i< count; ++i)
		{
			ClientConn *pClientConnRead = NULL;
			node *pHead = &m_listClientRead->head_node;
			for ((pNode)=(pHead)->next; (pHead) != (pNode);)
			{
				pClientConnRead = clientConnContain(pNode);
				if(FD_ISSET(pClientConnRead->socket, &fd_read))
				{
					int recvLen = receive(pClientConnRead->socket, m_recvBuf, m_recvBufLen);
					
					if(0 < recvLen)
					{
						if (recvLen == m_recvBufLen)
						{
							printf("receive may lost data!!!\n");
						}
						
						WORK_DATA *pWorkData = CDataWorkManager::instance()->createWorkData(recvLen);
						pWorkData->clientId = pClientConnRead->clientId;
						memcpy(pWorkData->m_pContent, m_recvBuf, recvLen);
						CDataWorkManager::instance()->pushWorkData(pWorkData);					
					}
					//异常处理
					else
					{					
						dealException(pClientConnRead->clientId);
						resetClientId(pClientConnRead->clientId);
						base::close(pClientConnRead->socket);
						
						pNode = m_listClientRead->erase(pNode);						
						base::free(pClientConnRead);
						continue;
					}
				}
				(pNode)=(pNode)->next;
			}
			
			//处理连接消息
			if(FD_ISSET(m_sockLister, &fd_read))
			{
				SOCKET socket = accept(m_sockLister,(sockaddr*)&clientAddr,&nLen);					
				ClientConn *pClientConn = (ClientConn *)base::malloc(sizeof(ClientConn));
				pClientConn->socket = socket;
				pClientConn->clientId = creatClientId();
				setClientId(pClientConn->clientId);
				m_listClientRead->push_back(&pClientConn->node);
			}
		}
	}
	return NULL;
}

void CNetServer::dealException(int clientId)
{
	char dispAllData[128];
	int dispAllDataLen;
	base::snprintf(dispAllData, sizeof(dispAllData), dataFormat, "dispAll", 0, 0, "", "", 0, "");
	dispAllDataLen = strlen(dispAllData);
	
	WORK_DATA *pWorkData = CDataWorkManager::instance()->createWorkData(dispAllDataLen);
	pWorkData->clientId = clientId;
	memcpy(pWorkData->m_pContent, dispAllData, dispAllDataLen);
	CDataWorkManager::instance()->pushWorkData(pWorkData);	
	return ;
}



int CNetServer::receive(SOCKET fd,char *szText,int len)
{
	int rc;
	rc=recv(fd,szText,len,0);
	if(rc <= 0)
	{
		return -1;
	}
	return rc;
}

int CNetServer::send(SOCKET fd,char *szText,int len)
{
	int cnt;
	int rc;
	cnt=len;
	while(cnt>0)
	{
		rc=::send(fd,szText,cnt,0);
		if(rc==SOCKET_ERROR)
		{
			return -1;
		}
		if(rc==0)
		{
			return len-cnt;
		}
		szText+=rc;
		cnt-=rc;
	}
	return len;
}

int CNetServer::creatClientId()
{
	if (m_newId < MAX_CLIENT_SIZE)
	{
		return m_newId++;
	}
	else
	{
		m_newId = 0;
		return m_newId;
	}
}

void CNetServer::setClientId(int clientId)
{
	m_cientIds.set(clientId);
	return ;
}

void CNetServer::resetClientId(int clientId)
{
	m_cientIds.reset(clientId);
	return ;
}

